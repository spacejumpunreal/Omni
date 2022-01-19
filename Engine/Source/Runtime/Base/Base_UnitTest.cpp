#include "Runtime/Prelude/Omni.h"
#include "External/googletest/include/gtest/gtest.h"
#include "Runtime/Base/Container/LinkedList.h"
#include "Runtime/Base/Container/Queue.h"
#include "Runtime/Base/Memory/MemoryArena.h"
#include "Runtime/Base/Misc/PrivateData.h"
#include "Runtime/Base/Memory/HandleObjectPoolImpl.h"
#include "Runtime/Base/Memory/ExternalAllocatorImpl.h"
#include <deque>
#include <random>

TEST(Base, Queue)
{
    Omni::Queue tq;
    EXPECT_TRUE(tq.IsEmpty());
    std::deque<Omni::SListNode> nodes;
    constexpr int               TestN = 10;
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

    constexpr u32    TestBatch = 16;
    constexpr u32    TestSingleSize = 1024;
    constexpr size_t TestMemSize = TestSingleSize * TestBatch;

    ScratchStack     stk;
    u8*              testMem = (u8*)malloc(TestMemSize);
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

class HandleObjectPoolTestObject
{
public:
    HandleObjectPoolTestObject(int x, float y, const std::string& z) : X(x), Y(y), Z(z)
    {
    }

public:
    int         X;
    float       Y;
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
        IndexHandle                 handle;
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
        IndexHandle                 handle;
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
        RawPtrHandle                handle;
        HandleObjectPoolTestObject* ptr;
        std::tie(handle, ptr) = pool.Alloc();
        new (ptr) HandleObjectPoolTestObject(1, 2.0f, "3");
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
        RawPtrHandle                handle;
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

namespace Omni
{
struct TestMemProvider0 final : public IExternalMemProvider
{
public:
    std::map<void*, u64> Allocated;
    u64                  BlockSize;
    u64                  TotalSize;

public:
    void Map(u64 size, u64 align, ExtenralAllocationBlockId& outBlockId) override
    {
        (void)align; //ignore align for now
        void* addr = malloc(size);
        memset(addr, 0, size);
        Allocated.insert({addr, size});
        outBlockId = addr;
        TotalSize += size;
    }
    void Unmap(ExtenralAllocationBlockId blockId) override
    {
        auto it = Allocated.find(blockId);
        TotalSize -= it->second;
        EXPECT_TRUE(it != Allocated.end());
        free(blockId);
    }
    u64 SuggestNewBlockSize(u64 reqSize) override
    {
        if (reqSize < BlockSize)
            return BlockSize;
        return reqSize;
    }

    PMRAllocator GetCPUAllocator() override
    {
        return {};
    }
};


struct TestExternalAllocator0 : public BestFitAllocator<TestMemProvider0>
{
public:
    using BestFitAllocator<TestMemProvider0>::mFreeMap;
    using BestFitAllocator<TestMemProvider0>::mFreeNodeList;

};


void CheckMem(u8* addr, u64 size, u8 pattern)
{
    for (u64 i = 0; i < size; ++i)
    {
        //EXPECT_EQ(addr[i], pattern);
        CheckAlways(addr[i] == pattern);
    }
}

void BeforeFree(void* addr, u64 size)
{
    CheckMem((u8*)addr, size, 1);
    memset(addr, 0, size);
}

void AfterAlloc(void* addr, u64 size)
{
    CheckMem((u8*)addr, size, 0);
    memset(addr, 1, size);
}

} // namespace Omni

TEST(Base, ExternalAllocator)
{
    using namespace Omni;
    TestExternalAllocator0 testAllocator;
    testAllocator.mProvider.BlockSize = 32 * 1024;
    testAllocator.mProvider.TotalSize = 0;
    {
        ExternalAllocation alloc0 = testAllocator.Alloc(16, 4);
        EXPECT_EQ(testAllocator.mFreeMap.size(), 1);
        ExternalAllocation alloc1 = testAllocator.Alloc(17, 4);
        EXPECT_EQ(testAllocator.mFreeMap.size(), 1);
        ExternalAllocation alloc2 = testAllocator.Alloc(18, 4);
        EXPECT_EQ(testAllocator.mFreeMap.size(), 2);
        testAllocator.Free(alloc0.Handle);
        testAllocator.Free(alloc1.Handle);
        testAllocator.Free(alloc2.Handle);
        EXPECT_EQ(testAllocator.mFreeMap.size(), 1);

        testAllocator.Cleanup();
        EXPECT_EQ(testAllocator.mProvider.TotalSize, 0);
    }

    {
        std::vector<ExternalAllocation>    allocs;
        u64                                total = 0;
        std::default_random_engine         generator(6);
        std::uniform_int_distribution<u64> distribution(1, 16);

        for (int i = 0; i < 1024 * 32; i++)
        {
            if (total > 5*1024)
            {
                u32 freeIdx = u32(allocs.size() / 2);
                BeforeFree((void*)(allocs[freeIdx].Start + (u8*)allocs[freeIdx].BlockId), allocs[freeIdx].Size);
                total -= allocs[freeIdx].Size;
                testAllocator.Free(allocs[freeIdx].Handle);
                std::swap(allocs[freeIdx], allocs.back());
                allocs.pop_back();
            }
            u64                rSize = distribution(generator);
            ExternalAllocation a0 = testAllocator.Alloc(rSize, 8);
            AfterAlloc(a0.Start + (u8*)a0.BlockId, a0.Size);
            total += a0.Size;
            allocs.push_back(a0);
        }
        for (ExternalAllocation& alloc : allocs)
        {
            BeforeFree((void*)(alloc.Start + (u8*)alloc.BlockId), alloc.Size);
            testAllocator.Free(alloc.Handle);
        }
        EXPECT_EQ(testAllocator.mFreeMap.size(), 1);

        testAllocator.Cleanup();
        EXPECT_EQ(testAllocator.mProvider.TotalSize, 0);
    }
    
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
