#include "JoinHandler.hpp"

JoinHandler::JoinHandler(const paxos::Ballot& current, const paxos::Data& data, int votersNumber) :
    current(current), data(data), votersNumber(votersNumber)
{
}

void JoinHandler::operator()(const paxos::AcceptorMsg& request, std::weak_ptr<PaxosSocket> socket)
{
    auto& logger = Poco::Logger::get("PaxosLogger");
    std::string voter = request.join().name();

    greatedVoters.erase(voter);
    seenVoters[voter] = socket;

    relaseGoneVoters(seenVoters);
    relaseGoneVoters(greatedVoters);

    logger.notice("Got join: " + voter +
        "; Seen " + std::to_string(seenVoters.size() + greatedVoters.size()) +
        " of " + std::to_string(votersNumber) + " voters");

    //decltype(seenVoters.size()) quorum = votersNumber/2 + 1;

    if(seenVoters.size() + greatedVoters.size() == (long unsigned int) votersNumber)
    {
        auto response = buildResponse();
        for(auto& voter : seenVoters)
        {
            auto paxosSocket = voter.second.lock();
            logger.notice("Sending welcome to: " + voter.first);
            paxosSocket->writeMessage(response);
            greatedVoters[voter.first] = voter.second;
        }
        seenVoters.clear();
    }
}

void JoinHandler::relaseGoneVoters(JoinHandler::VotersMap& voters)
{
    auto& logger = Poco::Logger::get("PaxosLogger");
    std::vector<std::string> goneVoters;
    for(auto& voter : voters)
    {
        if(voter.second.expired())
        {
            logger.notice("Lost voter : " + voter.first);
            goneVoters.push_back(voter.first);
        }
    }

    for(auto& voter : goneVoters)
    {
        voters.erase(voter);
    }
}

paxos::Welcome JoinHandler::buildWelcome()
{
    paxos::Welcome welcome{};
    welcome.mutable_current()->CopyFrom(current);
    welcome.mutable_value()->CopyFrom(data);
    return welcome;
}

paxos::ProposerMsg JoinHandler::buildResponse()
{
    paxos::ProposerMsg response{};
    response.set_discriminator(paxos::ProposerMsg::Welcome);
    response.mutable_welcome()->CopyFrom(buildWelcome());
    return response;
}
