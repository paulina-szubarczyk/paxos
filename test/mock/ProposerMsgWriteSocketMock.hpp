#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "messages.pb.h"
#include "PaxosSocketTypes.hpp"

using namespace testing;

struct ProposerMsgWriteSocketMock
{
    MOCK_METHOD1(writeProposerMsg, WriteStatus(const paxos::ProposerMsg&));

    template<typename Message>
    WriteStatus writeMessage(const Message& message);

};

template<>
WriteStatus ProposerMsgWriteSocketMock::writeMessage(const paxos::ProposerMsg& message);
