#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/Concurrency/LockfreeNodeCache.h"
#include "Runtime/Base/Math/CompileTime.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/MultiThread/IThreadLocal.h"
#include "Runtime/Base/MultiThread/LockfreeContainer.h"
#include "Runtime/Core/Allocator/MemoryModule.h"

namespace Omni
{
    //consts
    constexpr u32 LockfreeNodeMmapSize = 1024 * 1024;
    constexpr u32 LockfreeNodeTransferBatchCount = 32;
    constexpr u32 TmpCountSlot = 0;
    constexpr u32 TmpNextSlot = 1;
    

    //declarations
    class LockfreeNodeCachePerThreadData
    {
    public:
        LockfreeNodeCachePerThreadData();
        LockfreeNode* Allocate();
        void Free(LockfreeNode* node);
        void Cleanup();
        bool IsClean();
    private:
        u32					mHotCount;
        u32					mColdCount;
        LockfreeNode* mHotList;
        LockfreeNode* mColdList;
    };


    class LockfreeNodeCacheGlobalData
    {
    public:
        LockfreeNodeCacheGlobalData();
        std::tuple<LockfreeNode*, u32> AllocateBatch();
        void FreeBatch(LockfreeNode* lst, u32 count);
        void Cleanup();
        bool IsClean();
    private:
        LockfreeStack		mBatchStack;
    };


    //global data
    OMNI_DECLARE_THREAD_LOCAL(LockfreeNodeCachePerThreadData, gLockfreeNodeCachePerThreadData);
    static LockfreeNodeCacheGlobalData gLockfreeNodeCacheGlobalData;


    //LockfreeNodeCachePerThreadData impl
    LockfreeNodeCachePerThreadData::LockfreeNodeCachePerThreadData()
        : mHotCount(0)
        , mColdCount(0)
        , mHotList(nullptr)
        , mColdList(nullptr)
    {
    }

    LockfreeNode* LockfreeNodeCachePerThreadData::Allocate()
    {
        if (mHotCount == 0)
        {
            if (mColdCount == 0)
            {
                std::tie(mColdList, mColdCount) = gLockfreeNodeCacheGlobalData.AllocateBatch();
            }
            std::swap(mHotList, mColdList);
            std::swap(mHotCount, mColdCount);
        }
        --mHotCount;
        LockfreeNode* ret = mHotList;
        mHotList = (LockfreeNode*)mHotList->Next;
        ret->Next = nullptr;
        return ret;
    }

    void LockfreeNodeCachePerThreadData::Free(LockfreeNode* node)
    {
        if (mHotCount == LockfreeNodeTransferBatchCount)
        {
            ++mColdCount;
            node->Next = mColdList;
            mColdList = node;
            if (mColdCount == LockfreeNodeTransferBatchCount)
            {
                gLockfreeNodeCacheGlobalData.FreeBatch(mColdList, mColdCount);
                mColdCount = 0;
                mColdList = nullptr;
            }
        }
        else
        {
            ++mHotCount;
            node->Next = mHotList;
            mHotList = node;
        }
    }

    void LockfreeNodeCachePerThreadData::Cleanup()
    {
        if (mHotCount > 0)
            gLockfreeNodeCacheGlobalData.FreeBatch(mHotList, mHotCount);
        if (mColdCount > 0)
            gLockfreeNodeCacheGlobalData.FreeBatch(mColdList, mColdCount);
        mHotCount = 0;
        mColdCount = 0;
        mHotList = nullptr;
        mColdList = nullptr;
    }

    bool LockfreeNodeCachePerThreadData::IsClean()
    {
        return mHotCount == 0 && mColdCount == 0 && mHotList == nullptr && mColdList == nullptr;
    }

    //LockfreeNodeCacheGlobalData impl
    LockfreeNodeCacheGlobalData::LockfreeNodeCacheGlobalData()
    {
        CheckAlways(mBatchStack.Pop() == nullptr);
    }

    std::tuple<LockfreeNode*, u32> LockfreeNodeCacheGlobalData::AllocateBatch()
    {
        LockfreeNode* n = mBatchStack.Pop();
        if (n == nullptr)
        {
            constexpr u32 totalCount = LockfreeNodeMmapSize / sizeof(LockfreeNode);
            LockfreeNode* p = (LockfreeNode*)MemoryModule::Get().Mmap(LockfreeNodeMmapSize, LockfreeNodeMmapSize);
            constexpr u32 batch = totalCount / LockfreeNodeTransferBatchCount;
            LockfreeNode* tp = p;
            for (u32 iBatch = 0; iBatch < batch; ++iBatch)
            {
                for (u32 iNode = 0; iNode < LockfreeNodeTransferBatchCount; ++iNode)
                    tp[iNode].Next = &tp[iNode + 1];
                tp[LockfreeNodeTransferBatchCount - 1].Next = nullptr;
                if (iBatch != 0)
                    FreeBatch(tp, LockfreeNodeTransferBatchCount);
                tp = tp + (u64)LockfreeNodeTransferBatchCount;
            }
            return std::make_tuple(p, LockfreeNodeTransferBatchCount);
        }
        else
        {
            n->Next = (LockfreeNode*)n->Data[TmpNextSlot]; //n->Next filled will was used by mBatchStack for linked list, it was use for node freelist
            return std::make_tuple(n, (u32)(u64)n->Data[TmpCountSlot]);
        }
    }

    void LockfreeNodeCacheGlobalData::FreeBatch(LockfreeNode* lst, u32 count)
    {
        lst->Data[TmpCountSlot] = (void*)(u64)count;
        lst->Data[TmpNextSlot] = (LockfreeNode*)lst->Next; //n->Next filled will was used by mBatchStack for linked list, it was use for node freelist
        mBatchStack.Push(lst);
    }

    bool LockfreeNodeCacheGlobalData::IsClean()
    {
        return mBatchStack.Pop() == nullptr;
    }

    void LockfreeNodeCacheGlobalData::Cleanup()
    {
        std::unordered_set<u64, std::hash<u64>, std::equal_to<u64>> pages;
        while (true)
        {
            LockfreeNode* batch1 = mBatchStack.Pop();
            if (!batch1)
                break;
            LockfreeNode* batch = batch1;
            batch->Next = (LockfreeNode*)batch->Data[TmpNextSlot];
            u32 batchCount = (u32)(u64)batch->Data[TmpCountSlot];
            for (u32 i = 0; i < batchCount; ++i)
            {
                u64 addr = (u64)batch;
                addr >>= CompileTimeLog2(LockfreeNodeMmapSize);
                pages.insert(addr);
                batch = batch->Next;
            }
            CheckAlways(batch == nullptr);
        }
        for (u64 addr : pages)
        {
            MemoryModule::Get().Munmap((void*)(addr << CompileTimeLog2(LockfreeNodeMmapSize)), LockfreeNodeMmapSize);
        }
    }

    //LockfreeNodeCache impl
    void LockfreeNodeCache::GlobalInitialize()
    {
        CheckAlways(gLockfreeNodeCacheGlobalData.IsClean());
    }

    void LockfreeNodeCache::GlobalFinalize()
    {
        gLockfreeNodeCacheGlobalData.Cleanup();
    }

    void LockfreeNodeCache::ThreadInitialize()
    {
        CheckAlways(gLockfreeNodeCachePerThreadData.IsClean());
    }

    void LockfreeNodeCache::ThreadFinalize()
    {
        gLockfreeNodeCachePerThreadData.GetRaw().Cleanup();
    }

    LockfreeNode* LockfreeNodeCache::Alloc()
    {
        return gLockfreeNodeCachePerThreadData.GetRaw().Allocate();
    }

    void LockfreeNodeCache::Free(LockfreeNode* node)
    {
        gLockfreeNodeCachePerThreadData.GetRaw().Free(node);
    }

}
