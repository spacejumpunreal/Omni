#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Base/Container/LinkedList.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"
#include "Runtime/Core/GfxApi/DX12/DX12Fence.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"

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
        PMRVector<DX12TimelineManager::DX12RecycleCB>   Callbacks;
        
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
    {}

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
    u64 DX12TimelineManager::CloseBatchAndSignalOnGPU(GfxApiQueueType queueType, ID3D12CommandQueue* queue)
    {
        auto& self = mData.Ref<DX12TimelineManagerPrivateData>();
        auto& queueData = self.QueueData[(u8)queueType];
        LifeTimeBatch* newBatch = new LifeTimeBatch(queueData.Tail->BatchId + 1);
        u64 bid = queueData.Tail->BatchId;
        queueData.Tail->Next = newBatch;
        queueData.Tail = newBatch;
        UpdateFenceOnGPU(queueData.Fence, bid, queue);
        return bid;
    }
    void DX12TimelineManager::WaitBatchFinishOnGPU(GfxApiQueueType queueType, u64 batchId)
    {
        auto& self = mData.Ref<DX12TimelineManagerPrivateData>();
        auto& queueData = self.QueueData[(u8)queueType];
        ID3D12Fence* fence = queueData.Fence;
        CheckAlways(queueData.Head->BatchId <= batchId, "can not wait on future batch");
        //add waiting and checking
        while (true)
        {
            LifeTimeBatch*& batch = queueData.Head;
            u64 bid = batch->BatchId;
            auto& cbs = batch->Callbacks;
            WaitForFence(fence, bid);
            for (DX12RecycleCB& cb : cbs)
            {
                cb();
            }
            cbs.clear();
            LifeTimeBatch* next = (LifeTimeBatch*)batch->Next;
            CheckAlways(next != nullptr);
            batch = next;
            if (bid == batchId)
                break;
        }
    }
    bool DX12TimelineManager::IsBatchFinishedOnGPU(GfxApiQueueType queueType, u64 batchId)
    {
        auto& self = mData.Ref<DX12TimelineManagerPrivateData>();
        auto& queueData = self.QueueData[(u8)queueType];
        return queueData.Head->BatchId > batchId;
    }
    u64 DX12TimelineManager::AddBatchEvent(GfxApiQueueType queueType, DX12RecycleCB action)
    {
        auto& self = mData.Ref<DX12TimelineManagerPrivateData>();
        auto& queueData = self.QueueData[(u8)queueType];
        auto& cbs = queueData.Head->Callbacks;
        cbs.push_back(action);
        return queueData.Head->BatchId;
    }
}


#endif//OMNI_WINDOWS
