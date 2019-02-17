#include "AcceptorConnection.hpp"

AcceptorConnection::AcceptorConnection(
        const Poco::Net::StreamSocket& socket,
        Poco::Net::SocketReactor& reactor) :
    streamSocket(socket), reactor(reactor), paxosSocket(std::make_shared<PaxosSocket>(streamSocket))
{
    reactor.addEventHandler(streamSocket,
        Poco::Observer<AcceptorConnection, Poco::Net::ReadableNotification>
        (*this, &AcceptorConnection::read));
}

AcceptorConnection::~AcceptorConnection()
{
    unregisterFromReactor();
}

void AcceptorConnection::unregisterFromReactor()
{
    reactor.removeEventHandler(streamSocket,
        Poco::Observer<AcceptorConnection, Poco::Net::ReadableNotification>
        (*this, &AcceptorConnection::read));
}


void AcceptorConnection::checkStatus(const ReadStatus& status)
{
    logStatus(status);
    if(status == ReadStatus::GracefullShutdown)
    {
        unregisterFromReactor();
        delete this;
        return;
    }
}

void AcceptorConnection::read(Poco::Net::ReadableNotification* notification)
{
    notification->release();

    auto request = paxosSocket->readMessage<paxos::AcceptorMsg>();
    if(std::holds_alternative<paxos::AcceptorMsg>(request))
    {
        paxosMessageDispatcher->process(std::get<paxos::AcceptorMsg>(request), paxosSocket);
        return;
    }
    checkStatus(std::get<ReadStatus>(request));
}
