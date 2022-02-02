#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/GfxApi/GfxApiBlitPass.h"
#include "Runtime/Core/Allocator/MemoryModule.h"

namespace Omni
{

GfxApiBlitPass::GfxApiBlitPass(PageSubAllocator* alloc, u32 capacity) : CopyBufferCmdCount(capacity)
{
    CopyBufferCmds = alloc->AllocArray<GfxApiCopyBuffer>(capacity);
}

} // namespace Omni
