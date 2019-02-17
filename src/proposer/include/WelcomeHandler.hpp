#pragma once
#include "messages.pb.h"
#include "PaxosSocket.hpp"
#include <functional>
#include <memory>

class WelcomeHandler
{
public:
    using Callback = std::function<void(paxos::Ballot, paxos::Data)>;

    WelcomeHandler(const Callback& callback) : callback(callback) {}

    void operator()(const paxos::ProposerMsg& request, std::weak_ptr<PaxosSocket>)
    {
        auto welcome = request.welcome();
        callback(welcome.current(), welcome.value());
    }

private:
    Callback callback;
};

