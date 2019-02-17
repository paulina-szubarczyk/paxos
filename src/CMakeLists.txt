cmake_minimum_required(VERSION 3.5)
project(PaxosSrc)

find_package(Poco REQUIRED COMPONENTS Foundation Util Net)

add_subdirectory(protocol)
add_subdirectory(network)
add_subdirectory(acceptor)
add_subdirectory(proposer)
add_subdirectory(app)
