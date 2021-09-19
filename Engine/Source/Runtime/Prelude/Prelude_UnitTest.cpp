#include "Omni.h"
#include "MacroUtils.h"
#include <cstdio>
#include "gtest/gtest.h"

TEST(Prelude, Macro)
{
    EXPECT_EQ(OMNI_CONFIG_LIST(1, 13), 13);
    EXPECT_EQ(OMNI_CONFIG_LIST(0, 13, 1, 14), 14);
}

GTEST_API_ int main(int argc, char** argv) {
    printf("Running main() from %s\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

