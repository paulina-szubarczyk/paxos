add_library(PaxosLibrary STATIC Node.cpp)

target_include_directories(PaxosLibrary PUBLIC include)

target_link_libraries(PaxosLibrary PUBLIC Proposer Acceptor)


add_executable(Paxos main.cpp)

target_link_libraries(Paxos PUBLIC PaxosLibrary)
