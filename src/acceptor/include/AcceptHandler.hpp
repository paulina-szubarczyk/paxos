#pragma once
#include "messages.pb.h"
#include <optional>
#include <memory>

template<typename SocketType>
class AcceptHandler
{
public:
    AcceptHandler(const paxos::Ballot& promised, std::optional<paxos::Data>& data) :
        promised(promised), data(data)
    {
    }

    void operator()(const paxos::AcceptorMsg& request, std::weak_ptr<SocketType> socket)
    {
        auto accept = request.accept();
        if(accept.ballot().number() == promised.number())
        {
            data = accept.value();
            paxos::ProposerMsg response{};
            response.set_discriminator(paxos::ProposerMsg::Accepted);
            response.mutable_accepted()->mutable_ballot()->CopyFrom(accept.ballot());
            auto paxosSocket = socket.lock();
            paxosSocket->template writeMessage<paxos::ProposerMsg>(std::move(response));
        }
    }
private:
    const paxos::Ballot& promised;
    std::optional<paxos::Data>& data;
};
