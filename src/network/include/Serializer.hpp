#pragma once
#include "Poco/Logger.h"
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include "PaxosSocketTypes.hpp"

const int HEADER_SIZE = 4;

template<typename Message>
class MessageSerializer
{
public:
    MessageSerializer(const Message& msg): msg(msg){};

    void serialize()
    {
        int msgSize = msg.ByteSize();

        buffer.resize(msgSize+HEADER_SIZE, 0);
        google::protobuf::io::ArrayOutputStream aos(buffer.data(), (int)buffer.size());
        google::protobuf::io::CodedOutputStream coded_output(&aos);

        coded_output.WriteLittleEndian32(msgSize);
        msg.SerializeToCodedStream(&coded_output);
    }

    std::vector<char>& get()
    {
        return buffer;
    }

private:
    Message msg;
    std::vector<char> buffer;
};

class MessageSizeDeserializer
{
public:
    MessageSizeDeserializer() : header(HEADER_SIZE,0) {}
    std::vector<char>& buffer() { return header; }
    ReadStatus deserialize() {
        google::protobuf::io::ArrayInputStream ais(header.data(), (int)header.size());
        google::protobuf::io::CodedInputStream coded_input(&ais);
        coded_input.ReadLittleEndian32(&messageSize);
        if(messageSize <= 0)
            return ReadStatus::EncodingError;
        return ReadStatus::Success;
    }

    auto get()
    {
        return messageSize;
    }
private:
    std::vector<char> header;
    unsigned int messageSize;
};

template<typename Message>
class MessageDeserializer
{
public:
    MessageDeserializer(unsigned int size) : message(size, 0) {}
    std::vector<char>& buffer() { return message; }
    ReadStatus deserialize() {
        if(paxosMessage.ParseFromArray(message.data(), (int)message.size()))
        {
            return ReadStatus::Success;
        }
        return ReadStatus::EncodingError;
    }

    Message get() {
        return paxosMessage;
    }
private:
    std::vector<char> message;
    Message paxosMessage;
};
