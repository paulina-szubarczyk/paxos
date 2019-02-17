#include "Node.hpp"
#include "Poco/Logger.h"
#include <exception>

namespace
{
paxos::Data buildData(const std::string& nodeName, int value)
{
    paxos::Data data{};
    data.set_input(value);
    data.set_node_name(nodeName);
    return data;
}

}

Node::Node(NetworkTopology network):
    network(network),
    proposer(this->network, reactor),
    acceptor(this->network, reactor, buildData(network.seed.name, 100))
{
}

Node::~Node()
{
    reactor.stop();
    reactorThread.join();
}

bool Node::joinNetwork()
{
    auto& logger = Poco::Logger::get("PaxosLogger");
    reactorThread.start(reactor);
    if(proposer.join())
    {
        auto data = buildData(network.instantion.name, network.instantion.address.port()/100);
        proposer.propose(data);
        return true;
    }
    logger.error("Network not ready. Exiting");
    return false;
}

