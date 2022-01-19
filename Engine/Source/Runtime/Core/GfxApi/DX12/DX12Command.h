#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiRenderPass.h"
#include "Runtime/Core/GfxApi/GfxApiBlitPass.h"
namespace Omni
{
void DX12DrawRenderPass(GfxApiRenderPass* pass, GfxApiGpuEventRef* doneEvent);
void DX12CopyBlitPass(GfxApiBlitPass* pass);
} // namespace Omni

#endif // OMNI_WINDOWS
