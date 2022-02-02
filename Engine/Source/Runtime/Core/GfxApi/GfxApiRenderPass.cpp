#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/GfxApi/GfxApiRenderPass.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"

namespace Omni
{
GfxApiRenderPassStage::GfxApiRenderPassStage(PageSubAllocator* alloc, u32 capacity)
{
    Drawcalls = alloc->AllocArray<GfxApiDrawcall>(capacity);
}

GfxApiRenderPass::GfxApiRenderPass(PageSubAllocator* alloc, u32 stageCount) : mStageCount(stageCount)
{
    mStages = alloc->AllocArray<GfxApiRenderPassStage*>(mStageCount);
}

void GfxApiRenderPass::AddStage(u32 stageIndex, GfxApiRenderPassStage* stage)
{
    CheckAlways(stageIndex < mStageCount);
    GfxApiRenderPassStage* prevHead = mStages[stageIndex];
    stage->Next = prevHead;
    mStages[stageIndex] = stage;
}

} // namespace Omni
