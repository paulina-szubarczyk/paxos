#pragma once
#include "messages.pb.h"
#include "PaxosSocket.hpp"
#include <memory>
#include <type_traits>

template<typename Handler,
         typename RequestMessage,
         typename ResponseMessage = typename std::result_of<Handler(const RequestMessage&)>::type>
class ReactiveHandler
{
public:
    ReactiveHandler(const Handler& handler) : handler(handler) {}
    ReactiveHandler(){}
    void operator()(const RequestMessage& request, std::weak_ptr<PaxosSocket> socket)
    {
        auto paxosSocket = socket.lock();
        auto response = handler(request);
        paxosSocket->writeMessage<ResponseMessage>(std::move(response));
    }
private:
    Handler handler;
};
