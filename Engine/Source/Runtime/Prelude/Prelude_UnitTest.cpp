#include "Runtime/Prelude/Omni.h"
#include "External/googletest/include/gtest/gtest.h"
#include "Runtime/Prelude/MacroUtils.h"
#include <cstdio>

TEST(Prelude, Macro)
{
    EXPECT_EQ(OMNI_CONFIG_LIST(1, 13), 13);
    EXPECT_EQ(OMNI_CONFIG_LIST(0, 13, 1, 14), 14);
    EXPECT_EQ(OMNI_CONFIG_LIST(0, 1, 0, 13, 1, 14), 14);
    EXPECT_EQ(OMNI_CONFIG_LIST(0, 1, 0, 13, 1, 123, 0, 1, 0, 1), 123);
    EXPECT_EQ(OMNI_CONFIG_LIST(0, 1, 0, 13, 0, 1, 0, 1, 1, 1234), 1234);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

