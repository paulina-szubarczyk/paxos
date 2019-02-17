add_executable(paxostests
    test.cpp
    PaxosSocketTestSuite.cpp
    AcceptHandlerTestSuite.cpp
    PrepareHandlerTestSuite.cpp
    PromiseHandlerTestSuite.cpp
    PaxosConsensusTestSuite.cpp
    ProposerMsgWriteSocketMock.cpp)

target_include_directories(paxostests PUBLIC mock)
target_link_libraries(paxostests PUBLIC gtest gmock_main PaxosLibrary)
