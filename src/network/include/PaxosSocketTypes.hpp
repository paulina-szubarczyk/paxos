#pragma once

enum class ReadStatus
{
    GracefullShutdown,
    ReadError,
    EncodingError,
    Success
};

enum class WriteStatus
{
    WriteError,
    DecodingError,
    Success
};
