#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "messages.pb.h"
#include <google/protobuf/util/message_differencer.h>

#include "PromiseHandler.hpp"
#include "PaxosSocket.hpp"
#include "PaxosConsensus.hpp"

using namespace testing;

MATCHER_P(isBallot, ballot_number, "")
{
    return arg.number() == ballot_number;
}

MATCHER_P(isData, data, "")
{
    return arg.input() == data.input() and
           arg.node_name() == data.node_name();
}


MATCHER(isNulloptData, "")
{
    return not arg.has_value();
}

struct CallbackMock
{
    MOCK_METHOD2(call, void(paxos::Ballot, std::optional<paxos::Data>));
};

namespace
{
    std::string senderName = "testSender";
    int number_of_nodes = 1;
    int ballot_number = 100;
    int next_ballot_number = 110;
    int input = 200;
    std::string node_name = "test";

    paxos::Data buildData(int input, const std::string& name)
    {
        paxos::Data accepted_data;
        accepted_data.set_input(input);
        accepted_data.set_node_name(name);
        return accepted_data;
    }

    paxos::ProposerMsg buildPromiseResponse(int ballot_number)
    {
        paxos::ProposerMsg response;
        response.set_discriminator(paxos::ProposerMsg::Promise);
        response.mutable_promise()->mutable_promised()->set_number(ballot_number);
        return response;
    }

    paxos::ProposerMsg buildAcceptedResponse(int ballot_number)
    {
        paxos::ProposerMsg response;
        response.set_discriminator(paxos::ProposerMsg::Accepted);
        response.mutable_accepted()->mutable_ballot()->set_number(ballot_number);
        return response;
    }
}

struct SenderMock
{
    SenderMock(std::string name) : name(name) {}
    MOCK_METHOD1(propose, bool(int));
    MOCK_METHOD2(accept, bool(const paxos::Ballot&, const paxos::Data&));
    std::string name;
};

struct MessageDispatcherMock
{
    using MessageHandler = std::function<void(const paxos::ProposerMsg&, std::weak_ptr<PaxosSocket>)>;
    MOCK_METHOD2(registerHandler, void(int discriminator, const MessageHandler& handler));
};

struct PaxosConsensusTestSuite : public Test
{
    std::map<std::string, SenderMock> senders;
    std::shared_ptr<MessageDispatcherMock> messageDispatcher{std::make_shared<StrictMock<MessageDispatcherMock>>()};
    std::shared_ptr<PaxosSocket> paxosSocket;

    MessageDispatcherMock::MessageHandler promiseHandler;
    MessageDispatcherMock::MessageHandler preemptedHandler;
    MessageDispatcherMock::MessageHandler acceptedHandler;

    void saveRegisteredHandlers()
    {
        EXPECT_CALL(*messageDispatcher, registerHandler(paxos::ProposerMsg::Promise, _)).WillOnce(SaveArg<1>(&promiseHandler));
        EXPECT_CALL(*messageDispatcher, registerHandler(paxos::ProposerMsg::Preempted, _)).WillOnce(SaveArg<1>(&preemptedHandler));
        EXPECT_CALL(*messageDispatcher, registerHandler(paxos::ProposerMsg::Accepted, _)).WillOnce(SaveArg<1>(&acceptedHandler));
    }

    bool promiseBallotWithoutData(int ballot)
    {
        auto promise = buildPromiseResponse(ballot);
        promiseHandler(promise, paxosSocket);
        return true;
    }

    bool acceptBallot(const paxos::Ballot& ballot, const paxos::Data&)
    {
        auto accepted = buildAcceptedResponse(ballot.number());
        acceptedHandler(accepted, paxosSocket);
        return true;
    }
};

using namespace google::protobuf::util;
TEST_F(PaxosConsensusTestSuite, registersToMessageDispatcher)
{
    saveRegisteredHandlers();
    PaxosConsensus<SenderMock, MessageDispatcherMock> paxosConsensus{senders, messageDispatcher, number_of_nodes};
}

TEST_F(PaxosConsensusTestSuite, sendProposeToAllSenders)
{
    senders.emplace(senderName, senderName);
    saveRegisteredHandlers();
    PaxosConsensus<SenderMock, MessageDispatcherMock> paxosConsensus{senders, messageDispatcher, number_of_nodes};

    auto data = buildData(input, node_name);
    EXPECT_CALL(senders.at(senderName), propose(next_ballot_number))
        .WillOnce(Invoke(this, &PaxosConsensusTestSuite::promiseBallotWithoutData));
    EXPECT_CALL(senders.at(senderName), accept(isBallot(next_ballot_number), isData(data)))
        .WillOnce(Invoke(this, &PaxosConsensusTestSuite::acceptBallot));
    paxosConsensus.propose(data, ballot_number);

}
