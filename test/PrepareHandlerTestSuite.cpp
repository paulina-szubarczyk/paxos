#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "messages.pb.h"
#include <google/protobuf/util/message_differencer.h>

#include "PrepareHandler.hpp"
#include "ProposerMsgWriteSocketMock.hpp"

using namespace testing;

MATCHER_P(IsPromiseMessage, ballot, "")
{
    return arg.discriminator() == paxos::ProposerMsg::Promise and
           arg.promise().promised().number() == ballot.number() and
           arg.promise().has_value() == false;
}

MATCHER_P2(IsPromiseWithDataMessage, ballot, data, "")
{
    return arg.discriminator() == paxos::ProposerMsg::Promise and
           arg.promise().promised().number() == ballot.number() and
           arg.promise().has_value() and
           google::protobuf::util::MessageDifferencer::Equals(arg.promise().value(), data);
}

MATCHER_P(IsPreemptedMessage, ballot, ",")
{
    return arg.discriminator() == paxos::ProposerMsg::Preempted and
           arg.preempted().promised().number() == ballot.number() and
           arg.preempted().has_value() == false;
}

MATCHER_P2(IsPreemptedWithDataMessage, ballot, data, ",")
{
    return arg.discriminator() == paxos::ProposerMsg::Preempted and
           arg.preempted().promised().number() == ballot.number() and
           arg.preempted().has_value() and
           google::protobuf::util::MessageDifferencer::Equals(arg.preempted().value(), data);
}

namespace
{
    paxos::AcceptorMsg buildPrepareRequest(int ballot_number)
    {
        paxos::AcceptorMsg request;
        request.set_discriminator(paxos::AcceptorMsg::Prepare);
        request.mutable_prepare()->mutable_proposed()->set_number(ballot_number);
        return request;
    }

    paxos::Data buildData(int input, const std::string& name)
    {
        paxos::Data accepted_data;
        accepted_data.set_input(input);
        accepted_data.set_node_name(name);
        return accepted_data;
    }

    int input = 200;
    std::string node_name = "test";
    int join_ballot_number = 1;
    int prepare_ballot_number = 100;
}

struct PrepareHandlerTestSuite : public Test
{
    std::optional<paxos::Data> data;
    paxos::Ballot promised;
    PrepareHandler handler{promised, data};
};

using namespace google::protobuf::util;
TEST_F(PrepareHandlerTestSuite, promisesBallotWithoutDataForFirstPrepareReceived)
{
    promised.set_number(join_ballot_number);
    auto request = buildPrepareRequest(prepare_ballot_number);

    auto response = handler(request);
    EXPECT_THAT(response, IsPromiseMessage(request.prepare().proposed()));
    ASSERT_TRUE(MessageDifferencer::Equals(promised, request.prepare().proposed()));
}

TEST_F(PrepareHandlerTestSuite, promisesBallotWithDataForBallotHigherThenPromisedBallot)
{
    promised.set_number(prepare_ballot_number);

    auto accepted_data = buildData(input, node_name);
    data = accepted_data;

    auto request = buildPrepareRequest(prepare_ballot_number + 100);

    auto response = handler(request);
    EXPECT_THAT(response, IsPromiseWithDataMessage(request.prepare().proposed(), accepted_data));
    ASSERT_TRUE(MessageDifferencer::Equals(promised, request.prepare().proposed()));
}

TEST_F(PrepareHandlerTestSuite, respondsWithPreemptedForBallotLowerThenPromisedWithoutDataIfNeitherAccepted)
{
    promised.set_number(prepare_ballot_number);

    paxos::Ballot old_promise;
    old_promise.CopyFrom(promised);

    auto request = buildPrepareRequest(prepare_ballot_number - 10);

    auto response = handler(request);
    EXPECT_THAT(response, IsPreemptedMessage(old_promise));
    ASSERT_TRUE(MessageDifferencer::Equals(promised, old_promise));
}

TEST_F(PrepareHandlerTestSuite, respondsWithPreemptedForBallotLowerThenPromisedWithoutDataIfAnyAccepted)
{
    promised.set_number(prepare_ballot_number);

    auto accepted_data = buildData(input, node_name);
    data = accepted_data;

    paxos::Ballot old_promise;
    old_promise.CopyFrom(promised);

    auto request = buildPrepareRequest(prepare_ballot_number - 10);

    auto response = handler(request);
    EXPECT_THAT(response, IsPreemptedWithDataMessage(old_promise, accepted_data));
    ASSERT_TRUE(MessageDifferencer::Equals(promised, old_promise));
}
