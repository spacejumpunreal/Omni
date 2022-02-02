#include "Runtime/Core/CorePCH.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/Memory/MemoryArena.h"
#include "Runtime/Base/Math/SepcialFunctions.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/GfxApiUtils.h"
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
using DX12DeleteCBBatchMap = PMRUnorderedMap<GfxApiQueueMask, // key
    DX12DeleteCBBatch*,                                       // value
    std::hash<GfxApiQueueMask>,                               // hasher
    std::equal_to<GfxApiQueueMask>                            // equal_to
    >;

/**
 * declarations
 */
struct DX12DeleteManagerPrivateData
{
public:
    // DX12DeleteManagerPrivateData();

public:
    DX12DeleteCBBatchMap mKey2Batch{MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi)};
    u32                  mInFlightBatches{0};
};

using DX12DeleteManagerImpl = PImplCombine<DX12DeleteManager, DX12DeleteManagerPrivateData>;

DX12DeleteManager* DX12DeleteManager::Create()
{
    return OMNI_NEW(MemoryKind::GfxApi) DX12DeleteManagerImpl();
}

void DX12DeleteManager::Destroy()
{
    auto* self = DX12DeleteManagerImpl::GetCombinePtr(this);
    for (auto& kvp : self->mKey2Batch)
    {
        CheckAlways(kvp.second == nullptr);
    }
    OMNI_DELETE(self, MemoryKind::GfxApi);
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

void DX12DeleteManager::Flush()
{
    auto&                self = *DX12DeleteManagerImpl::GetCombinePtr(this);
    DX12TimelineManager* tm = gDX12GlobalState.TimelineManager;
    for (auto& kvp : self.mKey2Batch)
    {
        ++self.mInFlightBatches;
        GfxApiQueueMask                  mask = kvp.first;
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
            u32              typeCount = DecodeQueueTypeMask(mask, tps);
            tm->AddMultiQueueBatchCallback(tps, typeCount, cb);
            stk.Pop();
        }
    }
}

void DX12DeleteManager::OnBatchDelete()
{
    auto& self = *DX12DeleteManagerImpl::GetCombinePtr(this);
    --self.mInFlightBatches;
}

void DX12DeleteManager::AddDeleteCB(DX12DeleteCB cb, GfxApiQueueMask queueMask)
{
    CheckDebug(queueMask != 0);
    auto& self = *DX12DeleteManagerImpl::GetCombinePtr(this);
    auto& batchPtr = self.mKey2Batch[queueMask];
    if (batchPtr == nullptr)
        batchPtr =
            OMNI_NEW(MemoryKind::GfxApi) DX12DeleteCBBatch(MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi));
    batchPtr->push_back(cb);
}

} // namespace Omni

#endif // OMNI_WINDOWS
