#pragma once
#include <memory>
#include "PaxosSocket.hpp"
#include "messages.pb.h"
#include "Poco/Logger.h"

class ProposerSender
{
public:
    ProposerSender(std::weak_ptr<PaxosSocket> paxosSocket) : paxosSocket(paxosSocket)
    {}

    bool join(std::string name)
    {
        paxos::AcceptorMsg acceptorMessage{};
        acceptorMessage.set_discriminator(paxos::AcceptorMsg::Join);
        acceptorMessage.mutable_join()->set_name(name);
        return write(std::move(acceptorMessage));
    }

    bool propose(int ballot)
    {
        paxos::AcceptorMsg acceptorMessage{};
        acceptorMessage.set_discriminator(paxos::AcceptorMsg::Prepare);
        acceptorMessage.mutable_prepare()->mutable_proposed()->set_number(ballot);
        return write(std::move(acceptorMessage));
    }

    bool accept(const paxos::Ballot& ballot, const paxos::Data& value)
    {
        paxos::AcceptorMsg acceptorMessage{};
        acceptorMessage.set_discriminator(paxos::AcceptorMsg::Accept);
        acceptorMessage.mutable_accept()->mutable_ballot()->CopyFrom(ballot);
        acceptorMessage.mutable_accept()->mutable_value()->CopyFrom(value);
        return write(std::move(acceptorMessage));
    }

private:
    std::weak_ptr<PaxosSocket> paxosSocket;

    bool write(const paxos::AcceptorMsg& acceptorMessage)
    {
        auto socket = paxosSocket.lock();
        if(not socket)
        {
            auto& logger = Poco::Logger::get("PaxosLogger");
            logger.error("Invalid sender");
            return false;
        }
        auto written = socket->writeMessage(std::move(acceptorMessage));
        return written == WriteStatus::Success;
    }
};

