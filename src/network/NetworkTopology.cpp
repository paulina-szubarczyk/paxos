#include "NetworkTopology.hpp"
#include "Poco/Logger.h"

std::string toString(NodeConfig node)
{
    return "[" + node.name + ", " + node.address.toString() + "]";
}

void printNetworkTopology(NetworkTopology network)
{
    auto& logger = Poco::Logger::get("PaxosLogger");
    logger.notice("Running node: " + toString(network.instantion));

    for(auto& node : network.peers)
    {
        logger.information("Peer node: " + toString(node));
    }

    logger.notice("Seed node: " + toString(network.seed));

}

bool operator==(const NodeConfig& node1, const NodeConfig& node2)
{
    return node1.name == node2.name and node1.address == node2.address;
}

