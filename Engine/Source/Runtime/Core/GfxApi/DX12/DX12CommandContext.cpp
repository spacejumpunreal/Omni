#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12CommandContext.h"
#include "Runtime/Core/Allocator/MemoryModule.h"

namespace Omni
{
    /**
    * forward decl
    */

    /**
    * constants
    */

    /**
    * declarations
    */

    /**
    * definitions
    */
    DX12RenderPass::CacheFactory::CacheFactory()
    {}
    void* DX12RenderPass::CacheFactory::CreateObject()
    {
        return OMNI_NEW(MemoryKind::GfxApi)DX12RenderPass();
    }
    void DX12RenderPass::CacheFactory::DestroyObject(void* obj)
    {
        auto* pass = static_cast<DX12RenderPass*>(obj);
        OMNI_DELETE(pass, MemoryKind::GfxApi);
    }
    void DX12RenderPass::CacheFactory::RecycleCleanup(void* obj)
    {
        (void)obj;
    }
    
    DX12RenderPass::DX12RenderPass()
    {
    }

    DX12RenderPass::~DX12RenderPass()
    {
    }
}




#endif


