#include "Omni.h"
#include "Container/Queue.h"
#include "Container/LinkedList.h"
#include "gtest/gtest.h"
#include <deque>

TEST(Base, Queue)
{
    Omni::Queue tq;
    EXPECT_TRUE(tq.IsEmpty());
    std::deque<Omni::SListNode> nodes;
    constexpr int TestN = 10;
    for (int i = 0; i < TestN; ++i)
    {
        nodes.emplace_back(nullptr);
        Omni::SListNode* p = &nodes[i];
        tq.Enqueue(p, p);
    }
    
    for (int i = 0; i < TestN; ++i)
    {
        EXPECT_EQ(tq.Dequeue(), &nodes[i]);
    }
    EXPECT_TRUE(tq.IsEmpty());
}

int main(int argc, char** argv) {
    printf("Running main() from %s\n", __FILE__);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}