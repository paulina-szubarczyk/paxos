#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "messages.pb.h"
#include <google/protobuf/util/message_differencer.h>

#include "PromiseHandler.hpp"
#include "PaxosSocket.hpp"
#include "ProposerMsgWriteSocketMock.hpp"

using namespace testing;

MATCHER_P(isBallot, ballot_number, "")
{
    return arg.number() == ballot_number;
}

MATCHER_P(isData, data, "")
{
    return arg and
           arg->input() == data.input() and
           arg->node_name() == data.node_name();
}


MATCHER(isNulloptData, "")
{
    return arg.has_value() == false;
}

namespace
{
    paxos::ProposerMsg buildPromiseResponse(int ballot_number)
    {
        paxos::ProposerMsg response;
        response.set_discriminator(paxos::ProposerMsg::Promise);
        response.mutable_promise()->mutable_promised()->set_number(ballot_number);
        return response;
    }

    paxos::ProposerMsg buildPromiseResponse(int ballot_number, const paxos::Data& data)
    {
        auto response = buildPromiseResponse(ballot_number);
        response.mutable_promise()->mutable_value()->CopyFrom(data);
        return response;
    }

    paxos::ProposerMsg buildPreemptedResponse(int ballot_number)
    {
        paxos::ProposerMsg response;
        response.set_discriminator(paxos::ProposerMsg::Preempted);
        response.mutable_preempted()->mutable_promised()->set_number(ballot_number);
        return response;
    }

    paxos::ProposerMsg buildPreemptedResponse(int ballot_number, const paxos::Data& data)
    {
        auto response = buildPreemptedResponse(ballot_number);
        response.mutable_preempted()->mutable_value()->CopyFrom(data);
        return response;
    }

    paxos::Data buildData(int input, const std::string& name)
    {
        paxos::Data accepted_data;
        accepted_data.set_input(input);
        accepted_data.set_node_name(name);
        return accepted_data;
    }
    int three_responses_expected = 3;
    int promise_ballot_number = 100;
    int input = 123;
    std::string name = "test";
}

struct CallbackMock
{
    MOCK_METHOD2(call, void(paxos::Ballot, std::optional<paxos::Data>));
};

struct PromiseHandlerTestSuite : public Test
{
    StrictMock<CallbackMock> callbackMock;
    PromiseHandler::Callback callback;
    std::shared_ptr<PaxosSocket> socket{nullptr};
    void SetUp()
    {
        callback = [this](paxos::Ballot ballot, std::optional<paxos::Data> data)
        {
            callbackMock.call(ballot, data);
        };
    }
};

using namespace google::protobuf::util;
TEST_F(PromiseHandlerTestSuite, doesNothingWhenHaveNotReceivedAllRequiredResponses)
{
    PromiseHandler handler{callback, three_responses_expected};
    auto request = buildPromiseResponse(promise_ballot_number);

    handler(request, socket);
}

TEST_F(PromiseHandlerTestSuite, callsCallbackWithBallotFromPromiseWhenAllReceivedResponsesAreOfPromiseType)
{
    PromiseHandler handler{callback, three_responses_expected};

    auto first = buildPromiseResponse(promise_ballot_number);
    auto second = buildPromiseResponse(promise_ballot_number);
    auto third = buildPromiseResponse(promise_ballot_number);

    handler(first, socket);
    handler(second, socket);
    EXPECT_CALL(callbackMock, call(isBallot(promise_ballot_number), isNulloptData()));
    handler(third, socket);
}

TEST_F(PromiseHandlerTestSuite, callsCallbackWithBallotFromPromiseWithDataWhenAllReceivedResponsesAreOfPromiseType)
{
    PromiseHandler handler{callback, three_responses_expected};

    auto data = buildData(input, name);
    auto first = buildPromiseResponse(promise_ballot_number, data);
    auto second = buildPromiseResponse(promise_ballot_number, data);
    auto third = buildPromiseResponse(promise_ballot_number, data);

    handler(first, socket);
    handler(second, socket);
    EXPECT_CALL(callbackMock, call(isBallot(promise_ballot_number), isData(data)));
    handler(third, socket);
}

TEST_F(PromiseHandlerTestSuite, callsCallbackWithBallotAndDataFromHighestPreemptedReceived)
{
    PromiseHandler handler{callback, three_responses_expected};

    auto data = buildData(input + 20, name);
    auto first = buildPreemptedResponse(promise_ballot_number + 10, buildData(input+10, name));
    auto second = buildPreemptedResponse(promise_ballot_number + 20, data);
    auto third = buildPromiseResponse(promise_ballot_number, buildData(input, name));

    handler(first, socket);
    handler(second, socket);
    EXPECT_CALL(callbackMock, call(isBallot(promise_ballot_number + 20), isData(data)));
    handler(third, socket);
}

TEST_F(PromiseHandlerTestSuite, callsCallbackWithBallotFromHighestPreemptedReceived)
{
    PromiseHandler handler{callback, three_responses_expected};

    auto first = buildPreemptedResponse(promise_ballot_number + 10, buildData(input+10, name));
    auto second = buildPreemptedResponse(promise_ballot_number + 20);
    auto third = buildPromiseResponse(promise_ballot_number, buildData(input, name));

    handler(first, socket);
    handler(second, socket);
    EXPECT_CALL(callbackMock, call(isBallot(promise_ballot_number + 20), isNulloptData()));
    handler(third, socket);
}


