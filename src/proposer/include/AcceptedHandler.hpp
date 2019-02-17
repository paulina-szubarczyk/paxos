#pragma once
#include "messages.pb.h"
#include "Poco/Logger.h"
#include "PaxosSocket.hpp"
#include <vector>

class AcceptedHandler
{
public:
    using Callback = std::function<void(paxos::Ballot)>;
    AcceptedHandler(const Callback& callback, int response_number)
        : callback(callback), response_number(response_number) {}

    void operator()(const paxos::ProposerMsg& request, std::weak_ptr<PaxosSocket>)
    {
        requests.push_back(request);

        if(response_number > (int)requests.size())
        {
            return;
        }

        requests.push_back(request);
        callback(request.accepted().ballot());
    }
private:
    Callback callback;
    int response_number;
    std::vector<paxos::ProposerMsg> requests;
};

