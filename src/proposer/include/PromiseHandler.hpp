#pragma once
#include "messages.pb.h"
#include "Poco/Logger.h"
#include "PaxosSocket.hpp"
#include <vector>
#include <functional>
#include <memory>

class PromiseHandler
{
public:
    using Callback = std::function<void(paxos::Ballot, std::optional<paxos::Data>)>;

    PromiseHandler( const Callback& callback,
                   int response_number)
        : callback(callback), response_number(response_number) {}

    void operator()(const paxos::ProposerMsg& request, std::weak_ptr<PaxosSocket>)
    {
        requests.push_back(request);

        if(response_number > (int)requests.size())
        {
            return;
        }

        paxos::ProposerMsg* promise = nullptr;
        int ballot_number = -1;

        for(auto& request : requests)
        {
            if(request.discriminator() == paxos::ProposerMsg::Preempted and
               ballot_number <= request.preempted().promised().number())
            {
                promise = &request;
                ballot_number = request.preempted().promised().number();
            }
            else if(promise == nullptr)
            {
                promise = &request;
            };
        }

        if(promise == nullptr)
        {
            return;
        }

        paxos::Ballot ballot;
        std::optional<paxos::Data> data;
        if(promise->discriminator() == paxos::ProposerMsg::Preempted)
        {
            ballot = promise->preempted().promised();
            if(promise->preempted().has_value())
            {
                data = promise->preempted().value();
            }
        }
        else
        {
            ballot = promise->promise().promised();
            if(promise->promise().has_value())
            {
                data = promise->promise().value();
            }
        }

        callback(ballot, data);
    }

private:
    paxos::Ballot promised;
    paxos::Data value;
    Callback callback;
    int response_number;
    std::vector<paxos::ProposerMsg> requests;
};
