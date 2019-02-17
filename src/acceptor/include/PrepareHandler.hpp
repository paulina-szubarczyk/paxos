#pragma once
#include "Poco/Logger.h"
#include "messages.pb.h"
#include <memory>
#include <set>
#include <optional>

class PrepareHandler
{
public:
    PrepareHandler(paxos::Ballot& promised, const std::optional<paxos::Data>& data);

    paxos::ProposerMsg operator()(const paxos::AcceptorMsg& request);
private:
    paxos::Ballot& promised;
    const std::optional<paxos::Data>& data;
};

