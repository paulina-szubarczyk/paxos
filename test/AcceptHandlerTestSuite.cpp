#include "AcceptHandler.hpp"
#include "ProposerMsgWriteSocketMock.hpp"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "messages.pb.h"
#include <google/protobuf/util/message_differencer.h>

using namespace testing;

MATCHER_P(IsAcceptedMessage, ballot, "")
{
    return arg.discriminator() == paxos::ProposerMsg::Accepted and
           arg.accepted().ballot().number() == ballot.number();
}

namespace
{
    paxos::AcceptorMsg buildAcceptRequest(int ballot_number, int input, const std::string& node_name)
    {
        paxos::AcceptorMsg request;
        request.set_discriminator(paxos::AcceptorMsg::Accept);
        request.mutable_accept()->mutable_ballot()->set_number(ballot_number);
        request.mutable_accept()->mutable_value()->set_input(input);
        request.mutable_accept()->mutable_value()->set_node_name(node_name);
        return request;
    }

    int input = 200;
    std::string node_name = "test";
    int promised_ballot_number = 100;
}

struct AcceptHandlerTestSuite : public Test
{
    std::optional<paxos::Data> data;
    paxos::Ballot promised;
    std::shared_ptr<ProposerMsgWriteSocketMock> socket;
    AcceptHandler<ProposerMsgWriteSocketMock> handler{promised, data};

    void SetUp() override
    {
        socket = std::make_shared<StrictMock<ProposerMsgWriteSocketMock>>();
        promised.set_number(promised_ballot_number);
    }
};

TEST_F(AcceptHandlerTestSuite, doesAcceptPromisedBallot)
{
    auto request = buildAcceptRequest(promised.number(), input, node_name);

    EXPECT_CALL(*socket, writeProposerMsg(IsAcceptedMessage(promised))).WillOnce(Return(WriteStatus::Success));

    handler(request, socket);

    ASSERT_TRUE(data.has_value());
    ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(*data, request.accept().value()));
}

TEST_F(AcceptHandlerTestSuite, doesNotAcceptBallotLowerThenPromised)
{
    auto request = buildAcceptRequest(promised.number() - 10, input, node_name);

    handler(request, socket);

    ASSERT_FALSE(data.has_value());
}

TEST_F(AcceptHandlerTestSuite, doesNotAccepBallotHigherThenPromised)
{
    auto request = buildAcceptRequest(promised.number() + 10, input, node_name);

    handler(request, socket);

    ASSERT_FALSE(data.has_value());
}
