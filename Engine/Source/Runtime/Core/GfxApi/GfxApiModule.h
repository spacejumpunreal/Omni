#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Base/Misc/SharedObject.h"
#include "Runtime/Core/System/Module.h"
#include "Runtime/Core/GfxApi/GfxApiResource.h"

namespace Omni
{
    class GfxApiModule : public Module
    {
    public:
        static GfxApiModule& Get() { return *gGfxApiModule; }
      
        virtual SharedPtr<SharedObject> CreateResource(const GfxApiResourceDesc& desc) = 0;

        virtual GfxApiRenderPass* BeginRenderPass(const GfxApiRenderPassDesc& desc) = 0;
        virtual void EndRenderPass(const GfxApiRenderPass* desc) = 0;

        virtual GfxApiContext* BeginContext(const GfxApiContextDesc& desc) = 0; //for threaded recording
        virtual void EndContext(GfxApiContext* context) = 0;

        virtual GfxApiTexture* GetBackBuffer(u32 index) = 0;
        virtual u32 GetBackBufferCount() = 0;
        virtual u32 GetCurrentFrameIndex() = 0;
        virtual void Present() = 0;

    protected:
        static GfxApiModule* gGfxApiModule;
    };
}