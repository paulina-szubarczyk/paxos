#pragma once
#include "ProposerConnectionAcceptor.hpp"
#include "MessageDispatcher.hpp"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/SocketReactor.h"
#include "NetworkTopology.hpp"
#include "messages.pb.h"
#include "JoinHandler.hpp"
#include "ReactiveHandler.hpp"
#include "PrepareHandler.hpp"
#include "AcceptHandler.hpp"
#include <optional>
#include <memory>

class Acceptor
{
public:
    Acceptor(const NetworkTopology& network,
             Poco::Net::SocketReactor& reactor,
             const paxos::Data& data) :
        serverSocket(network.instantion.address),
        messageDispatcher(std::make_shared<AcceptorMessageDispatcher>()),
        proposerConnectionAcceptor(serverSocket, reactor, messageDispatcher)
    {
        promised.set_number(1);
        messageDispatcher->registerHandler(
            paxos::AcceptorMsg::Join,
            JoinHandler{promised, data, (int)network.peers.size()});

        messageDispatcher->registerHandler(paxos::AcceptorMsg::Prepare,
            ReactiveHandler<PrepareHandler, paxos::AcceptorMsg>{PrepareHandler{promised, acceptedData}});

        messageDispatcher->registerHandler(paxos::AcceptorMsg::Accept,
            AcceptHandler<PaxosSocket>{promised, acceptedData});
    }

private:
    Poco::Net::ServerSocket serverSocket;
    std::shared_ptr<AcceptorMessageDispatcher> messageDispatcher;
    ProposerConnectionAcceptor proposerConnectionAcceptor;
    std::optional<paxos::Data> acceptedData;
    paxos::Ballot promised;

};
