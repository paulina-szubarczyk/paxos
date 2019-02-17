#include "ProposerMsgWriteSocketMock.hpp"

template<>
WriteStatus ProposerMsgWriteSocketMock::writeMessage(const paxos::ProposerMsg& message)
{
    return writeProposerMsg(message);
}
