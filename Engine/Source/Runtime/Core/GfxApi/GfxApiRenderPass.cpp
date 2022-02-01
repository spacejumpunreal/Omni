#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/GfxApi/GfxApiRenderPass.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"

namespace Omni
{
GfxApiRenderPassStage::GfxApiRenderPassStage(u32 initialCount)
    : Drawcalls(initialCount, MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApiTmp))
{
}


GfxApiRenderPass::GfxApiRenderPass(u32 phaseCount) : mStageCount(phaseCount)
{
    PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApiTmp);
    mStages = alloc.allocate_object<GfxApiRenderPassStage*>(mStageCount);
}
GfxApiRenderPass::~GfxApiRenderPass()
{
    for (u32 iPhase = 0; iPhase < mStageCount; ++iPhase)
    {
        delete mStages[iPhase];
    }
    PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApiTmp);
    alloc.deallocate_object(mStages, mStageCount);
}
void GfxApiRenderPass::AddStage(u32 stageIndex, GfxApiRenderPassStage* stage)
{
    CheckAlways(stageIndex < mStageCount);
    GfxApiRenderPassStage* prevHead = mStages[stageIndex];
    stage->Next = prevHead;
    mStages[stageIndex] = stage;
}

} // namespace Omni
