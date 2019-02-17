#include "JSONNetworkTopologyParser.hpp"
#include "Poco/Net/SocketAddress.h"
#include "NetworkTopology.hpp"
#include <stdexcept>

namespace
{
    NodeConfig NULL_NODE{"", {"0.0.0.0", "0"}};
}

JSONNetworkTopologyParser::JSONNetworkTopologyParser(std::string jsonFile)
{
    jsonConfig = new Poco::Util::JSONConfiguration(jsonFile);
}

NetworkTopology JSONNetworkTopologyParser::parse(std::string instantionName)
{
    NetworkTopology networkTopology;

    networkTopology.nodesNumber = getNodesNumber();
    auto seedName = getSeedNodeName();

    for(int index=0; index < networkTopology.nodesNumber; ++index)
    {
        auto nodeConfig = getNode(index);
        if(nodeConfig == NULL_NODE)
        {
            throw std::invalid_argument("Error parsing config file");
        }
        if(nodeConfig.name == seedName)
            networkTopology.seed = nodeConfig;

        if(nodeConfig.name == instantionName)
            networkTopology.instantion = nodeConfig;

        networkTopology.peers.push_back(nodeConfig);
    }

    if(networkTopology.instantion == NULL_NODE)
    {
        printNetworkTopology(networkTopology);
        throw std::invalid_argument("Given instation name: " + instantionName + " not found.");
    }

    if(networkTopology.seed == NULL_NODE)
    {
        printNetworkTopology(networkTopology);
        throw std::invalid_argument("Given seed name: " + seedName + " not found.");
    }
    return networkTopology;
}

NodeConfig JSONNetworkTopologyParser::getNode(int index)
{
    std::string nodePrefix = "node[" + std::to_string(index) + "].";

    auto name = jsonConfig->getString(nodePrefix + "name");
    auto ip = jsonConfig->getString(nodePrefix + "ip");
    auto port = jsonConfig->getString(nodePrefix + "port");

    return NodeConfig{name, Poco::Net::SocketAddress{ip, port}};
}

int JSONNetworkTopologyParser::getNodesNumber()
{
    return jsonConfig->getInt("num_of_nodes");
}

std::string JSONNetworkTopologyParser::getSeedNodeName()
{
    return jsonConfig->getString("seed_name");
}
