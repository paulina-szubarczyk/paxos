#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "Poco/Net/StreamSocket.h"

int operation_flags{0};
using namespace testing;
struct PocoStreamSocketMock : public Poco::Net::StreamSocket
{
    MOCK_METHOD3(sendBytesImpl, int(const void*, int, int));
    int sendBytes(const void* buffer, int length, int flag = operation_flags)
    {
        message.resize(length, operation_flags);
        std::memcpy(message.data(), buffer, length);
        return sendBytesImpl(buffer, length, flag);
    }

    MOCK_METHOD3(receiveBytesImpl, int(void*, int, int));
    int receiveBytes(void* buffer, int length, int flag = operation_flags)
    {
        return receiveBytesImpl(buffer, length, flag);
    }

    int copyToBuffer(void* buffer, int length, int flag = operation_flags)
    {
        if(pos < message.size() and pos + length <= message.size())
        {
            std::memcpy(buffer, message.data() + pos, length);
            pos += length;
            return length;
        }
        return 0;
    }
    std::vector<char> message;
    int pos{0};
};
