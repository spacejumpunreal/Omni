#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Base/Container/LinkedList.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"

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
        u64 BatchId;
        PMRVector<DX12ObjectLifeTimeManager::DX12RecycleCB> Callbacks;
    };

    struct DX12ObjectLifeTimeManagerPrivateData
    {
    public:
        struct QueueData
        {
            LifeTimeBatch* Head;
            LifeTimeBatch* Tail;
        };
    public:
        QueueData QueueData[(u8)GfxApiQueueType::Count];
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

    //DX12ObjectLifeTimeManagerPrivateData


    //DX12ObjectLifeTimeManager
    DX12ObjectLifeTimeManager::DX12ObjectLifeTimeManager()
        : mData(PrivateDataType<DX12ObjectLifeTimeManagerPrivateData>{})
    {
        auto& self = mData.Ref<DX12ObjectLifeTimeManagerPrivateData>();
        for (u32 iQueue = 0; iQueue < (u8)GfxApiQueueType::Count; ++iQueue)
        {
            self.QueueData[iQueue].Head = self.QueueData[iQueue].Tail = new LifeTimeBatch(BatchIdInitValue);
        }
    }
    DX12ObjectLifeTimeManager::~DX12ObjectLifeTimeManager()
    {
        auto& self = mData.Ref<DX12ObjectLifeTimeManagerPrivateData>();
        for (u32 iQueue = 0; iQueue < (u8)GfxApiQueueType::Count; ++iQueue)
        {
            CheckAlways(
                self.QueueData[iQueue].Head == self.QueueData[iQueue].Tail,
                "DX12ObjectLifeTimeManager should only have empty batches opended on dtor");

            CheckAlways(
                self.QueueData[iQueue].Head->Callbacks.size() == 0,
                "DX12ObjectLifeTimeManager should only have empty batches opended on dtor");

            delete self.QueueData[iQueue].Head;
        }
        mData.DestroyAs<DX12ObjectLifeTimeManagerPrivateData>();
    }
    u64 DX12ObjectLifeTimeManager::CloseBatch(GfxApiQueueType queueType)
    {
        auto& self = mData.Ref<DX12ObjectLifeTimeManagerPrivateData>();
        auto& queueData = self.QueueData[(u8)queueType];
        LifeTimeBatch* newBatch = new LifeTimeBatch(queueData.Tail->BatchId + 1);
        u64 bid = queueData.Tail->BatchId;
        queueData.Tail->Next = newBatch;
        queueData.Tail = newBatch;
        return bid;
    }
    void DX12ObjectLifeTimeManager::OnBatchFinished(GfxApiQueueType queueType, u64 batchId)
    {
        auto& self = mData.Ref<DX12ObjectLifeTimeManagerPrivateData>();
        auto& queueData = self.QueueData[(u8)queueType];
        CheckAlways(batchId == queueData.Head->BatchId);
        auto& cbs = queueData.Head->Callbacks;
        for (DX12RecycleCB& cb : cbs)
        {
            cb();
        }
        LifeTimeBatch* next = (LifeTimeBatch*)queueData.Head->Next;
        CheckAlways(next != nullptr);
        queueData.Head = next;
    }
    u64 DX12ObjectLifeTimeManager::AddEvent(GfxApiQueueType queueType, DX12RecycleCB action)
    {
        auto& self = mData.Ref<DX12ObjectLifeTimeManagerPrivateData>();
        auto& queueData = self.QueueData[(u8)queueType];
        auto& cbs = queueData.Head->Callbacks;
        cbs.push_back(action);
        return queueData.Head->BatchId;
    }
}


#endif//OMNI_WINDOWS
