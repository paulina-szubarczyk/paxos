#pragma once
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketConnector.h"
#include "ProposerConnection.hpp"
#include "MessageDispatcher.hpp"
#include <memory>
#include <future>

class ProposerConnector : public Poco::Net::SocketConnector<ProposerConnection>
{
public:
    ProposerConnector(
            Poco::Net::SocketAddress& address,
            Poco::Net::SocketReactor& reactor,
            const std::shared_ptr<ProposerMessageDispatcher>& messsageDispatcher) :
        Poco::Net::SocketConnector<ProposerConnection>(address, reactor),
        messsageDispatcher(messsageDispatcher),
        senderProposerPromise(),
        senderProposerFuture(senderProposerPromise.get_future())
    {
    }

    std::future<ProposerSender> getSender()
    {
        return std::move(senderProposerFuture);
    }

protected:
    virtual ProposerConnection* createServiceHandler() override
    {
        auto proposerConnection = new ProposerConnection(socket(), *reactor());
        proposerConnection->init(messsageDispatcher, std::move(senderProposerPromise));
        return proposerConnection;
    }

    std::shared_ptr<ProposerMessageDispatcher> messsageDispatcher;
    std::promise<ProposerSender> senderProposerPromise;
    std::future<ProposerSender> senderProposerFuture;
};
