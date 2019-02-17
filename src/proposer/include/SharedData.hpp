#pragma once
#include <future>
#include <chrono>
#include "messages.pb.h"

struct PaxosData
{
    paxos::Ballot ballot;
    std::optional<paxos::Data> data;
};

class SharedData
{
public:
    std::optional<PaxosData> waitForResponse()
    {
        if(futureData.wait_for(timeout) == std::future_status::ready)
        {
            return futureData.get();
        }
        return std::nullopt;
    }

    void setResponse(PaxosData paxosData)
    {
        promiseData.set_value(paxosData);
    }

private:
    std::chrono::seconds timeout{20};
    std::promise<PaxosData> promiseData;
    std::future<PaxosData> futureData{promiseData.get_future()};

};
