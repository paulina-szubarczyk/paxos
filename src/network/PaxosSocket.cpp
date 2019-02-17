#include "Poco/Logger.h"
#include "PaxosSocket.hpp"


void logStatus(const ReadStatus& status)
{
    auto& logger = Poco::Logger::get("PaxosLogger");
    switch(status)
    {
    case ReadStatus::Success:
        logger.information("Read successfully");
        break;
    case ReadStatus::ReadError:
        logger.warning("Read error");
        break;
    case ReadStatus::EncodingError:
        logger.warning("Encoding error");
        break;
    case ReadStatus::GracefullShutdown:
        logger.notice("Gracefull shutdown");
        break;
    }
    return;
}

void logStatus(const WriteStatus& status)
{
    auto& logger = Poco::Logger::get("PaxosLogger");
    switch(status)
    {
    case WriteStatus::Success:
        logger.information("Write successfully");
        break;
    case WriteStatus::WriteError:
        logger.warning("Write error");
        break;
    case WriteStatus::DecodingError:
        logger.warning("Decoding error");
        break;
    }
    return;
}
