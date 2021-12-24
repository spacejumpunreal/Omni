#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiObject.h"

//forward decl

namespace Omni
{
    class DX12CommandRecorder
    {};

    class DX12RenderPass: public GfxApiRenderPass
    {
    public:
        DX12RenderPass();
        ~DX12RenderPass();
        void CleanupForRecycle() {}

        GfxApiCommandContext* BeginContext() override { return nullptr; };
        void EndContext(GfxApiCommandContext* ctx) override { (void)ctx; };
    private:

    };
}

#endif//OMNI_WINDOWS
