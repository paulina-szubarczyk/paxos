#pragma once
#include "Poco/AutoPtr.h"
#include "Poco/Util/JSONConfiguration.h"

struct NodeConfig;
struct NetworkTopology;

class JSONNetworkTopologyParser
{
public:
    JSONNetworkTopologyParser(std::string jsonFile);

    NetworkTopology parse(std::string instantionName);

private:
    NodeConfig getNode(int index);
    int getNodesNumber();
    std::string getSeedNodeName();

    Poco::AutoPtr<Poco::Util::JSONConfiguration> jsonConfig;
};
