#pragma once
#include "Poco/Logger.h"

#include <map>
#include <memory>

#include "PromiseHandler.hpp"
#include "AcceptedHandler.hpp"
#include "SharedData.hpp"

template<typename Sender, typename MessageDispatcher>
class PaxosConsensus
{
public:
    PaxosConsensus(std::map<std::string, Sender>& senders,
                   std::shared_ptr<MessageDispatcher> messageDispatcher,
                   int number_of_nodes)
        : senders(senders), messageDispatcher(messageDispatcher)
    {
        auto handler = std::make_shared<PromiseHandler>(
                PromiseHandler{
                    std::bind(&PaxosConsensus::promise,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2),
                    number_of_nodes
                    });

        auto promiseHandler =
            [handler] (const paxos::ProposerMsg& message, std::weak_ptr<PaxosSocket> socket)
            {
                (*handler)(message, socket);
            };

        messageDispatcher->registerHandler(paxos::ProposerMsg::Promise, promiseHandler);
        messageDispatcher->registerHandler(paxos::ProposerMsg::Preempted, promiseHandler);

        messageDispatcher->registerHandler(paxos::ProposerMsg::Accepted,
                AcceptedHandler{
                    std::bind(&PaxosConsensus::accepted,
                    this,
                    std::placeholders::_1),
                    number_of_nodes
                    });
    };

    bool propose(paxos::Data data, int ballot_number)
    {
        auto& logger = Poco::Logger::get("PaxosLogger");

        paxos::Ballot ballot = getNextBallot(ballot_number);
        bool consensus{false};
        do
        {
            proposeData = SharedData{};
            acceptedData = SharedData{};

            for(auto& sender : senders)
            {
                sender.second.propose(ballot.number());
            }

            auto promised = proposeData.waitForResponse();
            if(not promised)
            {
                logger.error("Haven't got all promised/preempted responses");
                break;
            }

            if(ballot.number() != promised->ballot.number())
            {
                logger.notice("Must re-propose ballot, because of preemption");
                ballot = getNextBallot(promised->ballot.number());
            }
            else
            {
                logger.notice("Going to second stage with my ballot");

                if(promised->data)
                {
                    data = *(promised->data);
                }
                else
                {
                    logger.notice("Going to send accept for my data");
                }

                for(auto& sender : senders)
                {
                    sender.second.accept(promised->ballot, data);
                }

                auto accepted = acceptedData.waitForResponse();
                if(not accepted)
                {
                    logger.error("Haven't got majority of acceptances");
                    logger.notice("Re-proposing");
                }
                else if(accepted->ballot.number() == promised->ballot.number())
                {
                    logger.notice("Got accepted.");
                    logger.notice(promised->ballot.DebugString());
                    logger.notice(data.DebugString());
                    consensus = true;
                }
            }
        }while(not consensus);
        return consensus;
    }
    void promise(paxos::Ballot ballot, std::optional<paxos::Data> data)
    {
        PaxosData paxosData{};
        paxosData.ballot = ballot;
        paxosData.data = data;
        proposeData.setResponse(paxosData);
    }

    void accepted(paxos::Ballot ballot)
    {
        PaxosData paxosData{};
        paxosData.ballot = ballot;
        acceptedData.setResponse(paxosData);
    }

private:
    std::map<std::string, Sender>& senders;
    std::shared_ptr<MessageDispatcher> messageDispatcher;
    SharedData proposeData;
    SharedData acceptedData;

    paxos::Ballot getNextBallot(int ballot_number)
    {
        paxos::Ballot ballot{};
        ballot.set_number(ballot_number + 10);
        return ballot;
    }
};

