add_library(Acceptor STATIC
            JoinHandler.cpp
            AcceptorConnection.cpp
            PrepareHandler.cpp)

target_include_directories(Acceptor PUBLIC include)

target_link_libraries(Acceptor PUBLIC Poco::Foundation Poco::Util Poco::Net)
target_link_libraries(Acceptor PUBLIC Network)

