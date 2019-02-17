#pragma once
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/SocketAcceptor.h"
#include "AcceptorConnection.hpp"
#include "MessageDispatcher.hpp"

class ProposerConnectionAcceptor : public Poco::Net::SocketAcceptor<AcceptorConnection>
{
public:
    ProposerConnectionAcceptor(
            Poco::Net::ServerSocket& serverSocket,
            Poco::Net::SocketReactor& reactor,
            const std::shared_ptr<AcceptorMessageDispatcher>& paxosReactor) :
        Poco::Net::SocketAcceptor<AcceptorConnection>(serverSocket, reactor),
        paxosReactor(paxosReactor)
    {
    }

protected:
    virtual AcceptorConnection* createServiceHandler(
        Poco::Net::StreamSocket& socket) override
    {
        auto acceptorConnection = new AcceptorConnection(socket, *reactor());
        acceptorConnection->setMessageReactor(paxosReactor);
        return acceptorConnection;
    }

    std::shared_ptr<AcceptorMessageDispatcher> paxosReactor;
};
