#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/Memory/HandleObjectPoolImpl.h"
#include "Runtime/Base/Memory/ExternalAllocation.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/WindowModule.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"
#include "Runtime/Core/GfxApi/GfxApiModule.h"
#include "Runtime/Core/GfxApi/GfxApiUtils.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12SwapChain.h"
#include "Runtime/Core/GfxApi/DX12/DX12Texture.h"
#include "Runtime/Core/GfxApi/DX12/DX12Buffer.h"
#include "Runtime/Core/GfxApi/DX12/DX12Shader.h"
#include "Runtime/Core/GfxApi/DX12/DX12Event.h"
#include "Runtime/Core/GfxApi/DX12/DX12GraphicsState.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12DeleteManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12BufferManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12Command.h"

#include <d3d12.h>
#include <dxgidebug.h>

#define DEBUG_DX_OBJECT_LEAK_ON_QUIT 1

EXTERN_C const GUID DECLSPEC_SELECTANY DXGI_DEBUG_ALL = {
    0xe48ae283, 0xda80, 0x490b, 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8};

namespace Omni
{
// forward decls
//
// declarations
class DX12Module final : public GfxApiModule
{
public:
    DX12Module(const EngineInitArgMap& args);
    void Destroy() override;
    void Initialize(const EngineInitArgMap&) override;
    void StopThreads() override;
    void Finalize() override;

#define GfxApiMethod(Definition) Definition override;
#include "Runtime/Core/GfxApi/GfxApiMethodList.inl"
#undef GfxApiMethod
private:
#if DEBUG_DX_OBJECT_LEAK_ON_QUIT
    void ReportAllLivingObjects();
#endif
};

/*
 * DX12Module implementations
 */

DX12Module::DX12Module(const EngineInitArgMap& args)
{
    (void)args;
}

void DX12Module::Initialize(const EngineInitArgMap& args)
{
    MemoryModule& mm = MemoryModule::Get();
    mm.Retain();

    CheckDebug(!gDX12GlobalState.Initialized);
    gDX12GlobalState.Initialize();

    Module::Initialize(args);
    CheckAlways(gGfxApiModule == nullptr);
    gGfxApiModule = this;
}

void DX12Module::StopThreads()
{
}

void DX12Module::Finalize()
{
    Module::Finalizing();
    if (GetUserCount() > 0)
        return;

    CheckAlways(gGfxApiModule != nullptr);
    gGfxApiModule = nullptr;

    gDX12GlobalState.Finalize();

    MemoryModule& mm = MemoryModule::Get();
    Module::Finalize();
    mm.Release();

#if DEBUG_DX_OBJECT_LEAK_ON_QUIT
    ReportAllLivingObjects();
#endif
}

/*
 * GfxApiMethod implementations
 */

// boilerplate
#define GFXAPI_OBJECT_REF(ObjectType) GfxApi##ObjectType##Ref
#define GFXAPI_OBJECT_DESC(ObjectType) GfxApi##ObjectType##Desc
#define CREATE_OBJECT_FUNC(ObjectType) Create##ObjectType
#define DESTROY_OBJECT_FUNC(ObjectType) Destroy##ObjectType
#define DX12_OBJECT(ObjectType) DX12##ObjectType
#define DX12_OBJECT_POOL(ObjectType) DX12##ObjectType##Pool
#define DELAY_FREE_FUNC(ObjectType) FreeHandleFor##ObjectType

#define IMPLEMENT_GFXAPI_OBJECT_CREATE(ObjectType)                                                                     \
    GFXAPI_OBJECT_REF(ObjectType)                                                                                      \
    DX12Module::CREATE_OBJECT_FUNC(ObjectType)(const GFXAPI_OBJECT_DESC(ObjectType) & desc)                            \
    {                                                                                                                  \
        GFXAPI_OBJECT_REF(ObjectType) handle;                                                                          \
        DX12_OBJECT(ObjectType) * obj;                                                                                 \
        std::tie((GFXAPI_OBJECT_REF(ObjectType)::UnderlyingHandle&)handle, obj) =                                      \
            gDX12GlobalState.DX12_OBJECT_POOL(ObjectType).Alloc();                                                     \
        new ((void*)obj) DX12_OBJECT(ObjectType)(desc);                                                                \
        return handle;                                                                                                 \
    }

#define IMPLEMENT_GFXAPI_OBJECT_DIRECT_DESTROY(ObjectType)                                                             \
    void DX12Module::DESTROY_OBJECT_FUNC(ObjectType)(GFXAPI_OBJECT_REF(ObjectType) handle)                             \
    {                                                                                                                  \
        DX12_OBJECT(ObjectType)* obj = gDX12GlobalState.DX12_OBJECT_POOL(ObjectType).ToPtr(handle);                    \
        obj->~DX12_OBJECT(ObjectType)();                                                                               \
        gDX12GlobalState.DX12_OBJECT_POOL(ObjectType).Free(handle);                                                    \
    }

#define IMPLEMENT_GFXAPI_OBJECT_DELAYED_DESTROY(ObjectType)                                                            \
    void DELAY_FREE_FUNC(ObjectType)(void* p)                                                                          \
    {                                                                                                                  \
        auto& handle = *reinterpret_cast<GfxApiBufferRef::UnderlyingHandle*>(&p);                                      \
        gDX12GlobalState.DX12_OBJECT_POOL(ObjectType).ToPtr(handle)->~DX12_OBJECT(ObjectType)();                       \
        gDX12GlobalState.DX12_OBJECT_POOL(ObjectType).Free(handle);                                                    \
    }                                                                                                                  \
    void DX12Module::DESTROY_OBJECT_FUNC(ObjectType)(GFXAPI_OBJECT_REF(ObjectType) handle)                             \
    {                                                                                                                  \
        gDX12GlobalState.DeleteManager->AddForHandleFree(DELAY_FREE_FUNC(ObjectType),                                  \
            (GFXAPI_OBJECT_REF(ObjectType)::UnderlyingHandle&)handle,                                                  \
            kAllQueueMask);                                                                                             \
    }


// Buffer
IMPLEMENT_GFXAPI_OBJECT_CREATE(Buffer);
IMPLEMENT_GFXAPI_OBJECT_DELAYED_DESTROY(Buffer);
void* DX12Module::DX12Module::MapBuffer(GfxApiBufferRef handle, u32 offset, u32 size)
{
    DX12Buffer* buffer = gDX12GlobalState.DX12BufferPool.ToPtr(handle);
    return buffer->Map(offset, size);
}
void DX12Module::UnmapBuffer(GfxApiBufferRef handle, u32 offset, u32 size)
{
    DX12Buffer* buffer = gDX12GlobalState.DX12BufferPool.ToPtr(handle);
    buffer->Unmap(offset, size);
}


// Texture
GfxApiTextureRef DX12Module::CreateTexture(const GfxApiTextureDesc& desc)
{
    (void)desc;
    NotImplemented();
    return {};
}
void DX12Module::DestroyTexture(GfxApiTextureRef texture)
{
    (void)texture;
    NotImplemented();
}


// Shader
IMPLEMENT_GFXAPI_OBJECT_CREATE(Shader);
IMPLEMENT_GFXAPI_OBJECT_DELAYED_DESTROY(Shader);


// SwapChain
IMPLEMENT_GFXAPI_OBJECT_CREATE(SwapChain);
IMPLEMENT_GFXAPI_OBJECT_DELAYED_DESTROY(SwapChain);
void DX12Module::UpdateSwapChain(GfxApiSwapChainRef swapChain, const GfxApiSwapChainDesc& desc)
{
    gDX12GlobalState.WaitGPUIdle();
    DX12SwapChain* dx12SwapChain = gDX12GlobalState.DX12SwapChainPool.ToPtr(swapChain);
    dx12SwapChain->Update(desc);
}
void DX12Module::GetBackbufferTextures(GfxApiSwapChainRef swapChain, GfxApiTextureRef backbuffers[], u32 count)
{
    DX12SwapChain* dx12SwapChain = gDX12GlobalState.DX12SwapChainPool.ToPtr(swapChain);
    dx12SwapChain->GetBackbufferTextures(backbuffers, count);
}
u32 DX12Module::GetCurrentBackbufferIndex(GfxApiSwapChainRef swapChain)
{
    DX12SwapChain* dx12SwapChain = gDX12GlobalState.DX12SwapChainPool.ToPtr(swapChain);
    return dx12SwapChain->GetCurrentBackbufferIndex();
}


// GpuEvent
bool DX12Module::IsEventTriggered(GfxApiGpuEventRef gpuEvent)
{
    DX12GpuEvent* gEvent = gDX12GlobalState.DX12GpuEventPool.ToPtr(gpuEvent);
    return gDX12GlobalState.TimelineManager->IsBatchFinishedOnGPU(gEvent->QueueType, gEvent->BatchId);
}

void DX12Module::WaitEvent(GfxApiGpuEventRef gpuEvent)
{
    DX12GpuEvent* gEvent = gDX12GlobalState.DX12GpuEventPool.ToPtr(gpuEvent);
    gDX12GlobalState.TimelineManager->WaitBatchFinishOnGPU(gEvent->QueueType, gEvent->BatchId);
}

void DX12Module::DestroyEvent(GfxApiGpuEventRef gpuEvent)
{
    DX12GpuEvent* gEvent = gDX12GlobalState.DX12GpuEventPool.ToPtr(gpuEvent);
    gEvent->~DX12GpuEvent();
    gDX12GlobalState.DX12GpuEventPool.Free(gpuEvent);
}


// PSOSignature(RootSignature)
IMPLEMENT_GFXAPI_OBJECT_CREATE(PSOSignature);
IMPLEMENT_GFXAPI_OBJECT_DIRECT_DESTROY(PSOSignature);

//GraphicsState
IMPLEMENT_GFXAPI_OBJECT_CREATE(BlendState);
IMPLEMENT_GFXAPI_OBJECT_DELAYED_DESTROY(BlendState);
IMPLEMENT_GFXAPI_OBJECT_CREATE(RasterizerState);
IMPLEMENT_GFXAPI_OBJECT_DELAYED_DESTROY(RasterizerState);
IMPLEMENT_GFXAPI_OBJECT_CREATE(DepthStencilState);
IMPLEMENT_GFXAPI_OBJECT_DELAYED_DESTROY(DepthStencilState);

// AsyncActions
void DX12Module::DrawRenderPass(GfxApiRenderPass* renderPass)
{
    DX12DrawRenderPass(renderPass);
}

void DX12Module::DispatchComputePass(GfxApiComputePass* computePass)
{
    (void)computePass;
    NotImplemented();
}

void DX12Module::CopyBlitPass(GfxApiBlitPass* blitPass)
{
    DX12CopyBlitPass(blitPass);
}

void DX12Module::Present(GfxApiSwapChainRef swapChain, bool waitVSync)
{
    DX12SwapChain* dx12SwapChain = gDX12GlobalState.DX12SwapChainPool.ToPtr(swapChain);
    dx12SwapChain->Present(waitVSync);
}

GfxApiGpuEventRef DX12Module::ScheduleGpuEvent(GfxApiQueueType queueType)
{
    ID3D12CommandQueue* queue = gDX12GlobalState.Singletons.D3DQueues[(u32)queueType];
    u64                 batchId;
    gDX12GlobalState.TimelineManager->CloseBatchAndSignalOnGPU(queueType, queue, batchId, true);

    GfxApiGpuEventRef handle;
    DX12GpuEvent*     gpuEvent;
    std::tie((GfxApiGpuEventRef::UnderlyingHandle&)handle, gpuEvent) = gDX12GlobalState.DX12GpuEventPool.Alloc();
    new ((void*)gpuEvent) DX12GpuEvent(batchId, queueType);
    return handle;
}

void DX12Module::ScheduleCompleteHandler(Action1<void, void*> completeHandler, GfxApiQueueMask queueMask)
{
    GfxApiQueueType tps[(u32)GfxApiQueueType::Count];
    u32 tpc = DecodeQueueTypeMask(queueMask, tps);
    if (tpc == 1)
    {
        gDX12GlobalState.TimelineManager->AddBatchCallback(tps[0], completeHandler);
    }
    else
    {
        gDX12GlobalState.TimelineManager->AddMultiQueueBatchCallback(tps, tpc, completeHandler);
    }
}


// Maintain operations
void DX12Module::CloseBatchDelete()
{
    gDX12GlobalState.DeleteManager->Flush();
}

void DX12Module::CheckGpuEvents(GfxApiQueueMask queueMask)
{
    gDX12GlobalState.TimelineManager->PollBatch(queueMask);
}

// Private functions

#if DEBUG_DX_OBJECT_LEAK_ON_QUIT
void DX12Module::ReportAllLivingObjects()
{
    IDXGIDebug* dxgiDebug;
    CheckSucceeded(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)));
    CheckSucceeded(dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL));
    // OmniDebugBreak();
    dxgiDebug->Release();
}
#endif

/*
 * DX12Module ctors
 */
static Module* DX12ModuleCtor(const EngineInitArgMap& args)
{
    return InitMemFactory<DX12Module>::New(args);
}

void DX12Module::Destroy()
{
    InitMemFactory<DX12Module>::Delete((DX12Module*)this);
}

EXPORT_INTERNAL_MODULE(DX12Module, ModuleExportInfo(DX12ModuleCtor, false, "DX12"));
} // namespace Omni

#endif // OMNI_WINDOWS
