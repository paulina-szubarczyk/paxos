#include <iostream>
#include "JSONNetworkTopologyParser.hpp"

#include "gtest/gtest.h"

TEST(Parser, throwsWhenConfigFileNotFound)
{
    ASSERT_THROW(JSONNetworkTopologyParser{""}, Poco::Exception);
}

TEST(Parser, throwsWhenSeedNameNotFound)
{
    ASSERT_THROW(JSONNetworkTopologyParser{""}, Poco::Exception);
}
