#include "Omni.h"
#include "MacroUtils.h"
#include <cstdio>
#include "gtest/gtest.h"

TEST(Prelude, Macro)
{
    EXPECT_EQ(OMNI_CONFIG_LIST(1, 13), 13);
    EXPECT_EQ(OMNI_CONFIG_LIST(0, 13, 1, 14), 14);
    EXPECT_EQ(OMNI_CONFIG_LIST(0, 1, 0, 13, 1, 14), 14);
    EXPECT_EQ(OMNI_CONFIG_LIST(0, 1, 0, 13, 1, 123, 0, 1, 0, 1), 123);
    EXPECT_EQ(OMNI_CONFIG_LIST(0, 1, 0, 13, 0, 1, 0, 1, 1, 1234), 1234);
}

int main(int argc, char** argv) {
    printf("Running main() from %s\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

