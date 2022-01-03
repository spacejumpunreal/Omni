#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Base/Container/LinkedList.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Base/Math/SepcialFunctions.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"
#include "Runtime/Core/GfxApi/DX12/DX12Fence.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineUtils.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"

#include <d3d12.h>

#if OMNI_WINDOWS

namespace Omni
{
    /**
    * constants
    */
    const u64 BatchIdInitValue = 1;
    
    /**
    * forward decls
    */

    /**
    * declarations
    */
    struct LifeTimeBatch : public SListNode
    {
    public:
        void* operator new(size_t size);
        void operator delete(void* ptr, size_t size);
        LifeTimeBatch(u64 batchId);
    public:
        u64                                             BatchId;
        PMRVector<DX12TimelineManager::DX12BatchCB>   Callbacks;
        
    };

    struct DX12MultiQueueCallback
    {
    public:
        DEFINE_GFX_API_OBJECT_NEW_DELETE();
        DX12MultiQueueCallback(DX12TimelineManager::DX12BatchCB callback, u32 deps);
        static void Satisfy(DX12MultiQueueCallback* obj);
    private:
        DX12TimelineManager::DX12BatchCB    mCallback;
        u32                                 mDeps;
    };

    struct DX12TimelineManagerPrivateData
    {
    public:
        struct QueueData
        {
            LifeTimeBatch*  Head;
            LifeTimeBatch*  Tail;
            ID3D12Fence*    Fence;
        };
    public:
        QueueData       QueueData[(u8)GfxApiQueueType::Count];
    };

    /**
    * definitions
    */

    //LifeTimeBatch
    void* LifeTimeBatch::operator new(size_t size)
    {
        return MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi).allocate_bytes(size, alignof(LifeTimeBatch));
    }
    void LifeTimeBatch::operator delete(void* ptr, size_t size)
    {
        return MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi).deallocate_bytes(ptr, size, alignof(LifeTimeBatch));
    }
    LifeTimeBatch::LifeTimeBatch(u64 batchId)
        : SListNode(nullptr)
        , BatchId(batchId)
        , Callbacks(MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi))
    {}

    //DX12MultiQueueCallback
    DX12MultiQueueCallback::DX12MultiQueueCallback(DX12TimelineManager::DX12BatchCB callback, u32 deps)
        : mCallback(callback)
        , mDeps(deps)
    {}

    void DX12MultiQueueCallback::Satisfy(DX12MultiQueueCallback* obj)
    {
        --obj->mDeps;
        if (obj->mDeps == 0)
        {
            obj->mCallback();
            delete obj;
        }
    }


    //DX12TimelineManagerPrivateData


    //DX12TimelineManager
    DX12TimelineManager::DX12TimelineManager(ID3D12Device* dev)
        : mData(PrivateDataType<DX12TimelineManagerPrivateData>{})
    {
        auto& self = mData.Ref<DX12TimelineManagerPrivateData>();
        for (u32 iQueue = 0; iQueue < (u8)GfxApiQueueType::Count; ++iQueue)
        {
            self.QueueData[iQueue].Head = self.QueueData[iQueue].Tail = new LifeTimeBatch(BatchIdInitValue + 1);
            self.QueueData[iQueue].Fence = CreateFence(BatchIdResetValue, dev);
        }
    }
    DX12TimelineManager::~DX12TimelineManager()
    {
        auto& self = mData.Ref<DX12TimelineManagerPrivateData>();
        for (u32 iQueue = 0; iQueue < (u8)GfxApiQueueType::Count; ++iQueue)
        {
            CheckAlways(
                self.QueueData[iQueue].Head == self.QueueData[iQueue].Tail,
                "DX12TimelineManager should only have empty batches opended on dtor");

            CheckAlways(
                self.QueueData[iQueue].Head->Callbacks.size() == 0,
                "DX12TimelineManager should only have empty batches opended on dtor");

            ReleaseFence(self.QueueData[iQueue].Fence);
            delete self.QueueData[iQueue].Head;
        }
        mData.DestroyAs<DX12TimelineManagerPrivateData>();
    }
    bool DX12TimelineManager::CloseBatchAndSignalOnGPU(GfxApiQueueType queueType, ID3D12CommandQueue* queue, u64& batchId, bool force)
    {
        auto& self = mData.Ref<DX12TimelineManagerPrivateData>();
        auto& queueData = self.QueueData[(u8)queueType];
        batchId = queueData.Head->BatchId;
        if ((queueData.Head->Callbacks.size() == 0) && !force)
            return false;
        LifeTimeBatch* newBatch = new LifeTimeBatch(batchId + 1);
        queueData.Tail->Next = newBatch;
        queueData.Tail = newBatch;
        UpdateFenceOnGPU(queueData.Fence, batchId, queue);
        return true;
    }
    void DX12TimelineManager::WaitBatchFinishOnGPU(GfxApiQueueType queueType, u64 batchId)
    {
        auto& self = mData.Ref<DX12TimelineManagerPrivateData>();
        auto& queueData = self.QueueData[(u8)queueType];
        ID3D12Fence* fence = queueData.Fence;
        //add waiting and checking
        while (true)
        {
            LifeTimeBatch*& batch = queueData.Head;
            u64 bid = batch->BatchId;
            if (bid > batchId)
                return;
            CheckDebug(batch->Next != nullptr, "can not wait on batch that is still open");
            auto& cbs = batch->Callbacks;
            WaitForFence(fence, bid);
            for (DX12BatchCB& cb : cbs)
            {
                cb();
            }
            cbs.clear();
            LifeTimeBatch* next = (LifeTimeBatch*)batch->Next;
            CheckAlways(next != nullptr);
            delete batch;
            batch = next;
        }
    }
    bool DX12TimelineManager::IsBatchFinishedOnGPU(GfxApiQueueType queueType, u64 batchId)
    {
        auto& self = mData.Ref<DX12TimelineManagerPrivateData>();
        auto& queueData = self.QueueData[(u8)queueType];
        return queueData.Head->BatchId > batchId;
    }
    void DX12TimelineManager::PollBatch(GfxApiQueueMask mask)
    {
        auto& self = mData.Ref<DX12TimelineManagerPrivateData>();
        while (mask != 0)
        {
            u32 iQueue = Mathf::FindMostSignificant1Bit(mask);
            mask &= ~(GfxApiQueueMask(1) << iQueue);
            GfxApiQueueType queueType = (GfxApiQueueType)iQueue;
            auto& queueData = self.QueueData[iQueue];
            u64 completed = queueData.Fence->GetCompletedValue();
            WaitBatchFinishOnGPU(queueType, completed);
        }
    }
    u64 DX12TimelineManager::AddBatchCallback(GfxApiQueueType queueType, DX12BatchCB action)
    {
        auto& self = mData.Ref<DX12TimelineManagerPrivateData>();
        auto& queueData = self.QueueData[(u8)queueType];
        auto& cbs = queueData.Head->Callbacks;
        cbs.push_back(action);
        return queueData.Head->BatchId;
    }
    void DX12TimelineManager::AddMultiQueueBatchCallback(GfxApiQueueType queuesTypes[], u32 queueCount, DX12BatchCB action)
    {
        DX12MultiQueueCallback* ccb = new DX12MultiQueueCallback(action, queueCount);
        u32 mask = 0;
        for (u32 iQueue = 0; iQueue < queueCount; ++iQueue)
        {
            GfxApiQueueType queueType = queuesTypes[iQueue];
            CheckDebug(mask & (1 << (u32)queueType));
            AddBatchCallback(queueType, TimelineHelpers::CreateBatchCB(DX12MultiQueueCallback::Satisfy, ccb));
        }
    }
}


#endif//OMNI_WINDOWS
