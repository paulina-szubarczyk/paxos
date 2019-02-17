#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "messages.pb.h"
#include <google/protobuf/util/message_differencer.h>

#include "PaxosSocketTypes.hpp"
#include "PocoStreamSocketMock.hpp"
#include "PaxosSocket.hpp"

namespace {
int ballot_number = 468;
paxos::ProposerMsg prepareTestMessage()
{
    paxos::ProposerMsg message;
    message.set_discriminator(paxos::ProposerMsg::Accepted);
    message.mutable_accepted()->mutable_ballot()->set_number(ballot_number);
    return message;
}
}

using namespace testing;

struct PaxosSocketTestSuite : public Test
{
    NiceMock<PocoStreamSocketMock> socket;
    PaxosSocketImpl<PocoStreamSocketMock> paxosSocket{socket};
};

TEST_F(PaxosSocketTestSuite, returnsStatusWriteFailedWhenNumberOfWrittenBytesIsDiffrentThenLengthOfBuffer)
{
    auto message = prepareTestMessage();

    int length = 0;
    EXPECT_CALL(socket, sendBytesImpl(_,_,operation_flags))
        .WillOnce(DoAll(SaveArg<1>(&length), Return(length - 1)));

    EXPECT_EQ(paxosSocket.writeMessage(message), WriteStatus::WriteError);
}

TEST_F(PaxosSocketTestSuite, returnsStatusSuccessWhenNumberOfWrittenBytesIsSameAsLengthOfBuffer)
{
    auto message = prepareTestMessage();

    EXPECT_CALL(socket, sendBytesImpl(_,_,operation_flags))
        .WillOnce(ReturnArg<1>());

    EXPECT_EQ(paxosSocket.writeMessage(message), WriteStatus::Success);
}

TEST_F(PaxosSocketTestSuite, returnsStatusErrorDecodingWhenMessageIsNotSerializable)
{
    paxos::ProposerMsg message;

    EXPECT_EQ(paxosSocket.writeMessage(message), WriteStatus::DecodingError);
}

TEST_F(PaxosSocketTestSuite, returnsReadStatusReadErrorWhenReadReturnsNegativeValue)
{
    auto message = prepareTestMessage();

    EXPECT_CALL(socket, receiveBytesImpl(_,_,operation_flags)).WillOnce(Return(-1));

    auto result = paxosSocket.readMessage<paxos::ProposerMsg>();

    ASSERT_TRUE(std::holds_alternative<ReadStatus>(result));
    auto status = std::get<ReadStatus>(result);
    ASSERT_EQ(status, ReadStatus::ReadError);
}

TEST_F(PaxosSocketTestSuite, returnsReadStatusReadErrorWhenReadsZeroBytes)
{
    auto message = prepareTestMessage();

    EXPECT_CALL(socket, receiveBytesImpl(_,_,operation_flags)).WillOnce(Return(0));

    auto result = paxosSocket.readMessage<paxos::ProposerMsg>();

    ASSERT_TRUE(std::holds_alternative<ReadStatus>(result));
    auto status = std::get<ReadStatus>(result);
    ASSERT_EQ(status, ReadStatus::GracefullShutdown);
}

TEST_F(PaxosSocketTestSuite, returnsReadStatusEncodingErrorWhenReadSizeOfMessageIsEqualZero)
{
    auto message = prepareTestMessage();

    paxosSocket.writeMessage(message);

    EXPECT_CALL(socket, receiveBytesImpl(_,HEADER_SIZE,operation_flags))
        .WillOnce(Invoke([HEADER_SIZE](void* buffer, int, int)
            {
                std::fill((char*)buffer, (char*)buffer + HEADER_SIZE, 0);
                return HEADER_SIZE;
            }));

    auto result = paxosSocket.readMessage<paxos::ProposerMsg>();

    ASSERT_TRUE(std::holds_alternative<ReadStatus>(result));
    auto status = std::get<ReadStatus>(result);
    ASSERT_EQ(status, ReadStatus::EncodingError);
}

TEST_F(PaxosSocketTestSuite, keepsReadingWhenReceivesNumberOfBytesLessThenExpected)
{
    auto message = prepareTestMessage();

    paxosSocket.writeMessage(message);

    int first_chunk = HEADER_SIZE/2;
    int second_chunk = HEADER_SIZE - first_chunk;

    EXPECT_CALL(socket, receiveBytesImpl(_,_,operation_flags))
        .WillOnce(Invoke(&socket, &PocoStreamSocketMock::copyToBuffer));

    EXPECT_CALL(socket, receiveBytesImpl(_,second_chunk,operation_flags))
        .WillOnce(Invoke([this, second_chunk](void* buffer, int, int)
            {
                return socket.copyToBuffer(buffer, second_chunk);
            }));

    EXPECT_CALL(socket, receiveBytesImpl(_,HEADER_SIZE,operation_flags))
        .WillOnce(Invoke([this, first_chunk](void* buffer, int, int)
            {
                return socket.copyToBuffer(buffer, first_chunk);
            }));

    auto result = paxosSocket.readMessage<paxos::ProposerMsg>();

    ASSERT_TRUE(std::holds_alternative<paxos::ProposerMsg>(result));
    auto readMessage = std::get<paxos::ProposerMsg>(result);
    ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(message, readMessage));
}

TEST_F(PaxosSocketTestSuite, successfulyReceivesAndEncodesMessageFromValidInput)
{
    auto message = prepareTestMessage();

    paxosSocket.writeMessage(message);

    EXPECT_CALL(socket, receiveBytesImpl(_,_,operation_flags))
        .WillOnce(Invoke(&socket, &PocoStreamSocketMock::copyToBuffer));

    EXPECT_CALL(socket, receiveBytesImpl(_,HEADER_SIZE,operation_flags))
        .WillOnce(Invoke(&socket, &PocoStreamSocketMock::copyToBuffer));

    auto result = paxosSocket.readMessage<paxos::ProposerMsg>();

    ASSERT_TRUE(std::holds_alternative<paxos::ProposerMsg>(result));
    auto readMessage = std::get<paxos::ProposerMsg>(result);
    ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(message, readMessage));
}

TEST_F(PaxosSocketTestSuite, returnsReadStatusEncodingErrorWhenCannotEncodeMessage)
{
    auto message = prepareTestMessage();

    paxosSocket.writeMessage(message);

    EXPECT_CALL(socket, receiveBytesImpl(_,_,operation_flags))
        .WillOnce(Invoke([](void* buffer, int length, int)
            {
                std::fill((char*)buffer, (char*)buffer + length, 0);
                return length;
            }));

    EXPECT_CALL(socket, receiveBytesImpl(_,HEADER_SIZE,operation_flags))
        .WillOnce(Invoke(&socket, &PocoStreamSocketMock::copyToBuffer));

    auto result = paxosSocket.readMessage<paxos::ProposerMsg>();

    ASSERT_TRUE(std::holds_alternative<ReadStatus>(result));
    auto status = std::get<ReadStatus>(result);
    ASSERT_EQ(status, ReadStatus::EncodingError);
}


