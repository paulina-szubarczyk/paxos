#pragma once
#include "Poco/Net/StreamSocket.h"
#include "Poco/Logger.h"
#include "Serializer.hpp"
#include "PaxosSocketTypes.hpp"
#include <variant>
#include <mutex>

template<class StreamSocket>
class PaxosSocketImpl
{
public:
    PaxosSocketImpl(StreamSocket& socket) : socket(socket) {}

    template<typename Message>
    WriteStatus writeMessage(const Message& message)
    {
        auto& logger = Poco::Logger::get("PaxosLogger");
        std::lock_guard<std::mutex> quard(socketMutex);
        logger.debug("Sending");
        logger.debug(message.DebugString());

        MessageSerializer serializer(std::move(message));
        try
        {
            serializer.serialize();
        }
        catch(...)
        {
            return WriteStatus::DecodingError;
        }

        auto wrote = socket.sendBytes(serializer.get().data(), (int) serializer.get().size());

        if(wrote != (int) serializer.get().size())
        {
            return WriteStatus::WriteError;
        }
        return WriteStatus::Success;
    }

    template<typename Message>
    std::variant<ReadStatus, Message> readMessage()
    {
        auto& logger = Poco::Logger::get("PaxosLogger");
        std::lock_guard<std::mutex> quard(socketMutex);

        MessageSizeDeserializer header{};
        ReadStatus status = read(header.buffer());
        if(status != ReadStatus::Success)
        {
            return status;
        }

        status = header.deserialize();
        if(status != ReadStatus::Success)
        {
            return status;
        }

        MessageDeserializer<Message> message{header.get()};
        status = read(message.buffer());
        if(status != ReadStatus::Success)
        {
            return status;
        }

        status = message.deserialize();
        if(status != ReadStatus::Success)
        {
            return status;
        }

        logger.debug("Read");
        logger.debug(message.get().DebugString());

        return message.get();
    }

private:
    ReadStatus read(std::vector<char>& buffer)
    {

        int received = 0;
        int size = (int) buffer.size();
        while(received < size)
        {
            int temp = socket.receiveBytes(buffer.data() + received, size - received);
            if(temp == 0)
            {
                return ReadStatus::GracefullShutdown;
            }
            if(temp < 0)
            {
                return ReadStatus::ReadError;
            }
            received += temp;
        }

        return ReadStatus::Success;
    }

    StreamSocket& socket;
    std::mutex socketMutex;
};

void logStatus(const ReadStatus&);
void logStatus(const WriteStatus&);

using PaxosSocket = PaxosSocketImpl<Poco::Net::StreamSocket>;
