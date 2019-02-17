add_library(Proposer STATIC
            ProposerConnection.cpp)

target_include_directories(Proposer PUBLIC include)
target_link_libraries(Proposer PUBLIC Poco::Foundation Poco::Util Poco::Net)
target_link_libraries(Proposer PUBLIC Network)
