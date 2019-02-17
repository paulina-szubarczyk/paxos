#pragma once
#include "Poco/Logger.h"

#include "ProposerConnector.hpp"
#include "MessageDispatcher.hpp"
#include "NetworkTopology.hpp"
#include "ProposerSender.hpp"
#include "PaxosConsensus.hpp"
#include "WelcomeHandler.hpp"
#include "SharedData.hpp"

#include <map>
#include <memory>
#include <chrono>

class Proposer
{
public:
    Proposer(NetworkTopology& network, Poco::Net::SocketReactor& reactor) :
        network(network),
        reactor(reactor),
        messageDispatcher(std::make_shared<ProposerMessageDispatcher>()),
        paxosConsensus(senders, messageDispatcher, (int)network.peers.size())
    {
        messageDispatcher->registerHandler(paxos::ProposerMsg::Welcome,
            WelcomeHandler{
                std::bind(&Proposer::welcome,
                this,
                std::placeholders::_1,
                std::placeholders::_2)});
    }

    bool join()
    {
        auto [seedConnection, inserted] = connect(network.seed);

        if(not inserted)
        {
            return false;
        }

        auto joined = seedConnection->second.join(network.instantion.name);

        if(not joined)
        {
            return false;
        }

        auto welcomed = welcomeData.waitForResponse();
        if(not welcomed)
        {
            return false;
        }

        currentBallot = generateStartBallot(*welcomed);
        connectToPeers();
        return true;
    }

    bool propose(const paxos::Data& data)
    {
        return paxosConsensus.propose(data, currentBallot);
    }

    void welcome(paxos::Ballot ballot, paxos::Data data)
    {
        PaxosData paxosData{};
        paxosData.ballot = ballot;
        paxosData.data = data;
        welcomeData.setResponse(paxosData);
    }


private:
    NetworkTopology& network;
    Poco::Net::SocketReactor& reactor;
    using ProposerConnections = std::map<std::string, std::unique_ptr<ProposerConnector>>;
    using ProposerSenders = std::map<std::string, ProposerSender>;
    ProposerConnections connectorMap;
    ProposerSenders senders;
    std::shared_ptr<ProposerMessageDispatcher> messageDispatcher;
    PaxosConsensus<ProposerSender, ProposerMessageDispatcher> paxosConsensus;

    SharedData welcomeData;
    int currentBallot;

    const std::chrono::seconds connectionTimeout{5};

    std::pair<ProposerSenders::iterator, bool> connect(NodeConfig& peer)
    {
        auto connector =
            std::make_unique<ProposerConnector>(peer.address, reactor, messageDispatcher);

        auto senderFuture = connector->getSender();
        auto status = senderFuture.wait_for(std::chrono::seconds(connectionTimeout));

        auto& logger = Poco::Logger::get("PaxosLogger");
        if(status == std::future_status::ready)
        {
            connectorMap.emplace(peer.name, std::move(connector));
            logger.notice("Connected to " + peer.name);
            return senders.emplace(peer.name, senderFuture.get());
        }
        logger.warning("Could not connect to: " + peer.address.toString());
        return std::make_pair(senders.end(), false);
    }


    int generateStartBallot(PaxosData& paxosData)
    {
        return paxosData.ballot.number() + network.instantion.address.port()/10;
    }

    void connectToPeers()
    {
        for(auto& node : network.peers)
        {
            if(node.name == network.seed.name)
            {
                continue;
            }
            connect(node);
        }
    }
};
