#pragma once
#include "messages.pb.h"
#include "Poco/Logger.h"
#include <functional>
#include <optional>
#include <map>
#include <memory>
#include "PaxosSocket.hpp"


template<typename Message>
class MessageDispatcher
{
public:

    using Discriminator = int;
    using MessageHandler = std::function<void(const Message&, std::weak_ptr<PaxosSocket>)>;

    void registerHandler(int discriminator, const MessageHandler& handler)
    {
        handlers[discriminator] = handler;
    }

    void process(const Message& message, std::weak_ptr<PaxosSocket> socket)
    {
        auto discriminator = message.discriminator();
        auto handler = handlers.find(discriminator);
        if(handler == handlers.end())
        {
            return;
        }
        handler->second(message, socket);
    }
private:
    std::map<Discriminator, MessageHandler> handlers;
};

using ProposerMessageDispatcher = MessageDispatcher<paxos::ProposerMsg>;
using AcceptorMessageDispatcher = MessageDispatcher<paxos::AcceptorMsg>;
