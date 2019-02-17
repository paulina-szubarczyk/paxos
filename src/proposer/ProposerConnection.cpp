#include "ProposerConnection.hpp"
#include "messages.pb.h"
#include "Poco/Logger.h"


ProposerConnection::ProposerConnection(
        const Poco::Net::StreamSocket& socket,
        Poco::Net::SocketReactor& reactor) :
    streamSocket(socket),
    reactor(reactor),
    paxosSocket(std::make_shared<PaxosSocket>(streamSocket)),
    readableObserver(*this, &ProposerConnection::read),
    writableObserver(*this, &ProposerConnection::write),
    shutdownObserver(*this, &ProposerConnection::shutdown)
{
    reactor.addEventHandler(streamSocket, readableObserver);
    reactor.addEventHandler(streamSocket, writableObserver);
    reactor.addEventHandler(streamSocket, shutdownObserver);
}

void ProposerConnection::unregisterFromReactor()
{
    if(reactor.hasEventHandler(streamSocket, readableObserver))
    {
        reactor.removeEventHandler(streamSocket, readableObserver);
    }

    if(reactor.hasEventHandler(streamSocket, writableObserver))
    {
        reactor.removeEventHandler(streamSocket, writableObserver);
    }

    if(reactor.hasEventHandler(streamSocket, shutdownObserver))
    {
        reactor.removeEventHandler(streamSocket, readableObserver);
    }
}

ProposerConnection::~ProposerConnection()
{
    unregisterFromReactor();
}

void ProposerConnection::checkStatus(const ReadStatus& status)
{
    logStatus(status);
    if(status == ReadStatus::GracefullShutdown)
    {
        unregisterFromReactor();
        delete this;
        return;
    }
}

void ProposerConnection::read(Poco::Net::ReadableNotification* notification)
{
    notification->release();

    auto request = paxosSocket->readMessage<paxos::ProposerMsg>();
    if(std::holds_alternative<paxos::ProposerMsg>(request))
    {
         messsageDispatcher->process(std::get<paxos::ProposerMsg>(request), paxosSocket);
         return;
    }
    checkStatus(std::get<ReadStatus>(request));
}

void ProposerConnection::write(Poco::Net::WritableNotification* notification)
{
    notification->release();

    senderPromise.set_value(ProposerSender{paxosSocket});
    reactor.removeEventHandler(streamSocket, writableObserver);
}

void ProposerConnection::shutdown(Poco::Net::ShutdownNotification* notification)
{
    notification->release();
}
