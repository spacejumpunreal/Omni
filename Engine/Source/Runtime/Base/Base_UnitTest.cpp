#include "Runtime/Prelude/Omni.h"
#include "External/googletest/include/gtest/gtest.h"
#include "Runtime/Base/Container/LinkedList.h"
#include "Runtime/Base/Container/Queue.h"
#include "Runtime/Base/Memory/MemoryArena.h"
#include "Runtime/Base/Misc/PrivateData.h"
#include "Runtime/Base/Memory/HandleObjectPoolImpl.h"
#include <deque>

#if 0

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

TEST(Base, MemoryArena)
{
    using namespace Omni;

    constexpr u32 TestBatch = 16;
    constexpr u32 TestSingleSize = 1024;
    constexpr size_t TestMemSize = TestSingleSize * TestBatch;
    
    ScratchStack stk;
    u8* testMem = (u8*)malloc(TestMemSize);
    stk.Reset(testMem, TestMemSize);

    for (u32 batch = 0; batch < TestBatch; ++batch)
    {
        stk.Push();
        EXPECT_EQ(stk.GetUsedBytes(), batch * TestSingleSize);
        stk.Allocate(TestSingleSize);
    }
    for (u32 batch = TestBatch; batch > 0; --batch)
    {
        EXPECT_EQ(stk.GetUsedBytes(), batch * TestSingleSize);
        stk.Pop();
    }
    EXPECT_EQ(stk.GetUsedBytes(), 0u);
    stk.Cleanup();
    EXPECT_EQ(stk.IsClean(), true);

    free(testMem);
}

TEST(Base, PrivateData)
{
    struct alignas(64) MyStruct
    {
        char Data[128];
    };
    using namespace Omni;
    PrivateData<128, 64> TestData(PrivateDataType<MyStruct>{});
    TestData.DestroyAs<MyStruct>();
}

#endif

class HandleObjectPoolTestObject
{
public:
    HandleObjectPoolTestObject(int x, float y, const std::string& z)
        : X(x)
        , Y(y)
        , Z(z)
    {}
public:
    int X;
    float Y;
    std::string Z;
};



TEST(Base, IndexHandleObjectPool)
{
    using namespace Omni;

    IndexObjectPool<HandleObjectPoolTestObject> pool;
    pool.Initialize(std::pmr::get_default_resource(), 4);
    std::pmr::vector<IndexHandle> handles;
    
    for (int i = 0; i < 16; ++i)
    {
        IndexHandle handle;
        HandleObjectPoolTestObject* ptr;
        std::tie(handle, ptr) = pool.Alloc();
        handles.push_back(handle);
        EXPECT_EQ(ptr, pool.ToPtr(handle));
    }
    pool.Free(handles[12]);
    pool.Free(handles[14]);
    pool.Free(handles[13]);
    pool.Free(handles[15]);
    handles.pop_back();
    handles.pop_back();
    handles.pop_back();
    handles.pop_back();

    for (int i = 0; i < 16; ++i)
    {
        IndexHandle handle;
        HandleObjectPoolTestObject* ptr;
        std::tie(handle, ptr) = pool.Alloc();
        handles.push_back(handle);
        EXPECT_EQ(ptr, pool.ToPtr(handle));
    }

    for (auto handle : handles)
    {
        pool.Free(handle);
    }
    pool.Finalize();
}

TEST(Base, RawPtrHandleObjectPool)
{
    using namespace Omni;

    RawPtrObjectPool<HandleObjectPoolTestObject> pool;
    pool.Initialize(std::pmr::get_default_resource(), 4);
    std::pmr::vector<RawPtrHandle> handles;

    for (int i = 0; i < 16; ++i)
    {
        RawPtrHandle handle;
        HandleObjectPoolTestObject* ptr;
        std::tie(handle, ptr) = pool.Alloc();
        new (ptr)HandleObjectPoolTestObject(1, 2.0f, "3");
        handles.push_back(handle);
        EXPECT_EQ(ptr, pool.ToPtr(handle));
    }
    pool.Free(handles[12]);
    pool.Free(handles[14]);
    pool.Free(handles[13]);
    pool.Free(handles[15]);
    handles.pop_back();
    handles.pop_back();
    handles.pop_back();
    handles.pop_back();

    for (int i = 0; i < 16; ++i)
    {
        RawPtrHandle handle;
        HandleObjectPoolTestObject* ptr;
        std::tie(handle, ptr) = pool.Alloc();
        handles.push_back(handle);
        EXPECT_EQ(ptr, pool.ToPtr(handle));
    }

    for (auto handle : handles)
    {
        pool.Free(handle);
    }
    pool.Finalize();
}


int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
