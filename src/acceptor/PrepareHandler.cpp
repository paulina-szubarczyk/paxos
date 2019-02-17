#include "PrepareHandler.hpp"

PrepareHandler::PrepareHandler(paxos::Ballot& promised, const std::optional<paxos::Data>& data) :
    promised(promised), data(data)
{
}

paxos::ProposerMsg PrepareHandler::operator()(const paxos::AcceptorMsg& request)
{
    int proposed = request.prepare().proposed().number();
    if(proposed > promised.number())
    {
        promised.set_number(proposed);

        paxos::ProposerMsg response{};
        response.set_discriminator(paxos::ProposerMsg::Promise);
        response.mutable_promise()->mutable_promised()->CopyFrom(promised);
        if(data)
        {
            response.mutable_promise()->mutable_value()->CopyFrom(*data);
        }
        return response;
    }

    paxos::ProposerMsg response{};
    response.set_discriminator(paxos::ProposerMsg::Preempted);
    response.mutable_preempted()->mutable_promised()->CopyFrom(promised);
    if(data)
    {
        response.mutable_preempted()->mutable_value()->CopyFrom(*data);
    }
    return response;
}
