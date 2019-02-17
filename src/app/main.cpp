#include "Poco/Logger.h"
#include "Poco/ConsoleChannel.h"
#include "Poco/AutoPtr.h"

#include "JSONNetworkTopologyParser.hpp"
#include "NetworkTopology.hpp"
#include "Node.hpp"
#include <optional>

std::optional<NetworkTopology> processInputParameters(int argc, char *argv[])
try
{
    auto& logger = Poco::Logger::get("PaxosLogger");
    if(argc != 3)
    {
        logger.warning("Usage: ./Paxos json_network_config_file instantion_node_name");
        return std::nullopt;
    }

    std::string jsonFile = argv[1];
    std::string instantionNodeName = argv[2];

    JSONNetworkTopologyParser configParser(jsonFile);
    return configParser.parse(instantionNodeName);
}
catch(const Poco::Exception& e)
{
    auto& logger = Poco::Logger::get("PaxosLogger");
    logger.warning(e.what());
    return std::nullopt;
}
catch(const std::exception& e)
{
    auto& logger = Poco::Logger::get("PaxosLogger");
    logger.warning(e.what());
    return std::nullopt;
}

void setRootLogger()
{
    Poco::AutoPtr<Poco::ColorConsoleChannel> coutChannel(new Poco::ColorConsoleChannel);
    coutChannel->setProperty("informationColor", "blue");
    coutChannel->setProperty("noticeColor", "lightBlue");
    Poco::Logger::root().setChannel(coutChannel);
    Poco::Logger::root().setLevel("debug");
}

int main(int argc, char *argv[])
{
    setRootLogger();

    auto network = processInputParameters(argc, argv);

    if(not network)
    {
        auto& logger = Poco::Logger::get("PaxosLogger");
        logger.fatal("Configuration error. Exiting.");
        return 1;
    }

    printNetworkTopology(*network);
    Node node{std::move(*network)};

    if(not node.joinNetwork())
    {
        return 0;
    }

    while(true)
    {}

    return 0;
}
