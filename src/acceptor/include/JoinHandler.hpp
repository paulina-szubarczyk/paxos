#pragma once
#include "Poco/Logger.h"
#include "messages.pb.h"
#include "PaxosSocket.hpp"
#include <memory>
#include <set>

class JoinHandler
{
public:
    JoinHandler(const paxos::Ballot& current, const paxos::Data& data, int votersNumber);

    void operator()(const paxos::AcceptorMsg& request, std::weak_ptr<PaxosSocket> socket);
private:
    using VotersMap = std::map<std::string, std::weak_ptr<PaxosSocket>>;

    void relaseGoneVoters(VotersMap& voters);

    paxos::Welcome buildWelcome();

    paxos::ProposerMsg buildResponse();

    paxos::Ballot current;
    paxos::Data data;

    VotersMap seenVoters;
    VotersMap greatedVoters;

    int votersNumber;
};
