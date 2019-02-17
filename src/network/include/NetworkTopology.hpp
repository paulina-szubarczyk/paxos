#pragma once
#include <vector>
#include "Poco/Net/SocketAddress.h"

struct NodeConfig
{
    std::string name;
    Poco::Net::SocketAddress address;
};

struct NetworkTopology
{
    std::vector<NodeConfig> peers;
    int nodesNumber;
    NodeConfig seed;
    NodeConfig instantion;
};

std::string toString(NodeConfig node);
void printNetworkTopology(NetworkTopology network);
bool operator==(const NodeConfig& node1, const NodeConfig& node2);
