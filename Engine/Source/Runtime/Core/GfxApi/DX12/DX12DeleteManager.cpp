#include "Runtime/Core/CorePCH.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Base/Memory/MemoryArena.h"
#include "Runtime/Base/Math/SepcialFunctions.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12DeleteManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineUtils.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"



#if OMNI_WINDOWS

namespace Omni
{
    /**
    * constants
    */
    
    /**
    * forward decls
    */
    using DX12DeleteCBBatch = PMRVector<DX12DeleteManager::DX12DeleteCB>;
    using DX12DeleteCBBatchMap = PMRUnorderedMap< 
        GfxApiQueueMask,//key
        DX12DeleteCBBatch*,//value
        std::hash<GfxApiQueueMask>,//hasher
        std::equal_to<GfxApiQueueMask>//equal_to
    >;

    /**
    * declarations
    */
    struct DX12DeleteManagerPrivateData
    {
    public:
        DX12DeleteManagerPrivateData();
    public:
        DX12DeleteCBBatchMap        mKey2Batch;
        u32                         mInFlightBatches = 0;
    };
    

    DX12DeleteManagerPrivateData::DX12DeleteManagerPrivateData()
        : mKey2Batch(MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi))
    {}

    DX12DeleteManager::DX12DeleteManager()
        : mData(PrivateDataType<DX12DeleteManagerPrivateData>{})
    {
    }

    DX12DeleteManager::~DX12DeleteManager()
    {
        auto& self = mData.Ref<DX12DeleteManagerPrivateData>();
        for (auto& kvp : self.mKey2Batch)
        {
            CheckAlways(kvp.second == nullptr);
        }
        mData.DestroyAs<DX12DeleteManagerPrivateData>();
    }

    void RunBatchDelete(DX12DeleteCBBatch* batch)
    {
        for (DX12DeleteManager::DX12DeleteCB& cb : (*batch))
        {
            cb();
        }
        OMNI_DELETE(batch, MemoryKind::GfxApi);
        gDX12GlobalState.DeleteManager->OnBatchDelete();
    }

    void DX12DeleteManager::AddForHandleFree(void(*func)(void*), IndexHandle handle, GfxApiQueueMask queueMask)
    {
        void* p = *reinterpret_cast<void**>(&handle);
        DX12DeleteManager::DX12DeleteCB cb(func, p);
        AddDeleteCB(cb, queueMask);
    }

    void DX12DeleteManager::Flush()
    {
        auto& self = mData.Ref<DX12DeleteManagerPrivateData>();
        DX12TimelineManager* tm = gDX12GlobalState.TimelineManager;
        for (auto& kvp : self.mKey2Batch)
        {
            ++self.mInFlightBatches;
            GfxApiQueueMask mask = kvp.first;
            DX12TimelineManager::DX12BatchCB cb = TimelineHelpers::CreateBatchCB(RunBatchDelete, kvp.second);
            kvp.second = nullptr;
            if (Mathf::IsSingleBitSet(mask))
            {
                tm->AddBatchCallback((GfxApiQueueType)Mathf::FindMostSignificant1Bit(mask), cb);
            }
            else
            {
                ScratchStack& stk = MemoryModule::GetThreadScratchStack();
                stk.Push();
                GfxApiQueueType* tps = (GfxApiQueueType*)stk.Allocate(sizeof(GfxApiQueueType[(u32)GfxApiQueueType::Count]));
                u32 typeCount = 0;
                while (mask != 0)
                {
                    u32 bitIdx = Mathf::FindMostSignificant1Bit(mask);
                    tps[typeCount] = (GfxApiQueueType)bitIdx;
                    ++typeCount;
                    mask = mask & ~(GfxApiQueueMask(1) << bitIdx);
                }
                tm->AddMultiQueueBatchCallback(tps, typeCount, cb);
                stk.Pop();
            }
        }
    }

    void DX12DeleteManager::OnBatchDelete()
    {
        auto& self = mData.Ref<DX12DeleteManagerPrivateData>();
        --self.mInFlightBatches;
    }

    void DX12DeleteManager::AddDeleteCB(DX12DeleteCB cb, GfxApiQueueMask queueMask)
    {
        CheckDebug(queueMask != 0);
        auto& self = mData.Ref<DX12DeleteManagerPrivateData>();
        auto& batchPtr = self.mKey2Batch[queueMask];
        if (batchPtr == nullptr)
            batchPtr = OMNI_NEW(MemoryKind::GfxApi) DX12DeleteCBBatch(MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi));
        batchPtr->push_back(cb);
    }

}


#endif//OMNI_WINDOWS
