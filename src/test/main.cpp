#include "core.hpp"
#include "regex_test.hpp"
#include "scanner_test.hpp"
#include <gtest/gtest.h>
using namespace bee;

s32 main(s32 argc, char *argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
