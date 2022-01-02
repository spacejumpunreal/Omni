#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiRenderPass.h"
namespace Omni
{
    void DX12DrawRenderPass(GfxApiRenderPass* pass, GfxApiGpuEventRef* doneEvent);
}

#endif//OMNI_WINDOWS
