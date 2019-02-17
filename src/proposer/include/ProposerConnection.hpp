#pragma once
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketNotification.h"
#include "PaxosSocket.hpp"
#include "ProposerSender.hpp"
#include "MessageDispatcher.hpp"
#include "messages.pb.h"
#include <memory>
#include <future>

class ProposerConnection
{
public:
    ProposerConnection(
        const Poco::Net::StreamSocket& socket,
        Poco::Net::SocketReactor& reactor);

    ~ProposerConnection();

    void init(
        const std::shared_ptr<ProposerMessageDispatcher>& dispatcher,
        std::promise<ProposerSender> senderProposerPromise)
    {
        messsageDispatcher = dispatcher;
        senderPromise = std::move(senderProposerPromise);
    }

    void read(Poco::Net::ReadableNotification* notification);
    void write(Poco::Net::WritableNotification* notification);
    void shutdown(Poco::Net::ShutdownNotification* notification);
public:
    Poco::Net::StreamSocket streamSocket;
    Poco::Net::SocketReactor& reactor;
    std::shared_ptr<PaxosSocket> paxosSocket;
    Poco::Observer<ProposerConnection, Poco::Net::ReadableNotification> readableObserver;
    Poco::Observer<ProposerConnection, Poco::Net::WritableNotification> writableObserver;
    Poco::Observer<ProposerConnection, Poco::Net::ShutdownNotification> shutdownObserver;
    std::shared_ptr<ProposerMessageDispatcher> messsageDispatcher;
    std::promise<ProposerSender> senderPromise;

    void checkStatus(const ReadStatus& status);
    void unregisterFromReactor();

};
