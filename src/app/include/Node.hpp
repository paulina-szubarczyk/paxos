#pragma once
#include "Poco/Net/SocketReactor.h"
#include "Poco/Thread.h"

#include "NetworkTopology.hpp"
#include "Proposer.hpp"
#include "Acceptor.hpp"

#include <memory>

class Node
{
public:
    Node(NetworkTopology network);
    ~Node();

    bool joinNetwork();
private:
    Poco::Net::SocketReactor reactor;
    NetworkTopology network;
    Proposer proposer;
    Acceptor acceptor;
    Poco::Thread reactorThread;
};

