#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/GfxApi/GfxApiBlitPass.h"
#include "Runtime/Core/Allocator/MemoryModule.h"


namespace Omni
{


GfxApiBlitPass::GfxApiBlitPass(u32 reserveCopyCmds) 
    : CopyCmds(reserveCopyCmds, MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApiTmp))
{
}


} // namespace Omni
