#pragma once
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketNotification.h"
#include "PaxosSocket.hpp"
#include "MessageDispatcher.hpp"

class AcceptorConnection
{
public:
    AcceptorConnection(const Poco::Net::StreamSocket& socket,
             Poco::Net::SocketReactor& reactor);
    ~AcceptorConnection();
    void read(Poco::Net::ReadableNotification* notification);

    void setMessageReactor(
        const std::shared_ptr<AcceptorMessageDispatcher>& messageDispatcher)
    {
        paxosMessageDispatcher = messageDispatcher;
    }

public:
    Poco::Net::StreamSocket streamSocket;
    Poco::Net::SocketReactor& reactor;
    std::shared_ptr<PaxosSocket> paxosSocket;
    std::shared_ptr<AcceptorMessageDispatcher> paxosMessageDispatcher;

    void checkStatus(const ReadStatus& status);
    void unregisterFromReactor();
};
