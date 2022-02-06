#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/GfxApi/GfxApiRenderPass.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"

namespace Omni
{
GfxApiRenderPassStage::GfxApiRenderPassStage(PageSubAllocator* alloc, u32 capacity) : DrawcallCount(capacity)
{
    Drawcalls = alloc->AllocArray<GfxApiDrawcall>(capacity);
}

GfxApiRenderPass::GfxApiRenderPass(PageSubAllocator* alloc, u32 phaseCount) : PhaseCount(phaseCount)
{
    ZeroFill(*this);
    PhaseCount = phaseCount;
    PhaseArray = alloc->AllocArray<GfxApiRenderPassStage*>(phaseCount);
    memset(PhaseArray, 0, sizeof(GfxApiRenderPassStage*) * phaseCount);
}

void GfxApiRenderPass::AddStage(u32 stageIndex, GfxApiRenderPassStage* stage)
{
    CheckAlways(stageIndex < PhaseCount);
    GfxApiRenderPassStage* prevHead = PhaseArray[stageIndex];
    stage->Next = prevHead;
    PhaseArray[stageIndex] = stage;
}

u32 GfxApiRenderPass::GetStageCount() const
{
    u32 acc = 0;
    for (u32 iPhase = 0; iPhase < PhaseCount; ++iPhase)
    {
        for (GfxApiRenderPassStage* ptr = PhaseArray[iPhase]; ptr != nullptr; ptr = (GfxApiRenderPassStage*)ptr->Next)
        {
            ++acc;
        }
    }
    return acc;
}

} // namespace Omni
