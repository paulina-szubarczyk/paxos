add_library(Network STATIC
            NetworkTopology.cpp
            PaxosSocket.cpp
            JSONNetworkTopologyParser.cpp)

target_include_directories(Network PUBLIC include)

target_link_libraries(Network PUBLIC Poco::Foundation Poco::Util Poco::Net)

target_link_libraries(Network PUBLIC PaxosProtocol)
