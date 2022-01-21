#include "Runtime/Engine/EnginePCH.h"
#include "Runtime/Engine/Render/DemoRendererModule.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/Memory/MemoryArena.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Concurrency/JobPrimitives.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"
#include "Runtime/Core/GfxApi/GfxApiModule.h"
#include "Runtime/Core/GfxApi/GfxApiRenderPass.h"
#include "Runtime/Core/Platform/WindowModule.h"
#include "Runtime/Engine/Timing/TimingModule.h"

#define DEMO_MODULE 1

namespace Omni
{
/**
 * Constants
 */
constexpr u32 BackbufferCount = 3;

/**
 * Declarations
 */

struct DemoRendererModulePrivateImpl : public ICallback
{
public:
    DemoRendererModulePrivateImpl();
    void Tick();
    void operator()() override
    {
        Tick();
    }

public:
    DispatchWorkItem*   TickRegistry = nullptr;
    GfxApiSwapChainDesc DescSwapChain;
    GfxApiSwapChainRef  SwapChain = {};
    GfxApiTextureRef    Backbuffers[BackbufferCount];
    GfxApiBufferRef     TestUploadBuffer;
    GfxApiBufferRef     TestGPUBuffer;
    u32                 ClientAreaSizeX = 0;
    u32                 ClientAreaSizeY = 0;
    u32                 FrameIndex = 0;
};

using DemoRendererImpl = PImplCombine<DemoRendererModule, DemoRendererModulePrivateImpl>;

/**
 * constants
 */
static constexpr u32 DemoRendererTickPriority = 100;

/**
 * definitions
 */

DemoRendererModulePrivateImpl::DemoRendererModulePrivateImpl()
{
}

void DemoRendererModule::Initialize(const EngineInitArgMap& args)
{
    MemoryModule& mm = MemoryModule::Get();
    mm.Retain();
    WindowModule& wm = WindowModule::Get();
    wm.Retain();
    TimingModule& tm = TimingModule::Get();
    tm.Retain();
    GfxApiModule& gfxApi = GfxApiModule::Get();
    gfxApi.Retain();

    DemoRendererImpl& self = *DemoRendererImpl::GetCombinePtr(this);
    // create swapchain
    wm.GetClientAreaSize(self.ClientAreaSizeX, self.ClientAreaSizeY);
    self.DescSwapChain.BufferCount = BackbufferCount;
    self.DescSwapChain.Width = self.ClientAreaSizeX;
    self.DescSwapChain.Height = self.ClientAreaSizeY;
    self.DescSwapChain.Format = GfxApiFormat::R8G8B8A8_UNORM;
    self.DescSwapChain.WindowHandle = wm.GetMainWindowHandle();
    self.SwapChain = gfxApi.CreateSwapChain(self.DescSwapChain);
    gfxApi.GetBackbufferTextures(self.SwapChain, self.Backbuffers, BackbufferCount);

    {
        GfxApiBufferDesc bufferDesc;
        bufferDesc.Size = 16 * 1024;
        bufferDesc.AccessFlags = GfxApiAccessFlags::Upload;
        self.TestUploadBuffer = gfxApi.CreateBuffer(bufferDesc);
        u8* range = (u8*)gfxApi.MapBuffer(self.TestUploadBuffer, 0, 1024);
        memset(range, 0x88, 1024);
        gfxApi.UnmapBuffer(self.TestUploadBuffer, 0, 1024);

        bufferDesc.Size = 32 * 1024;
        bufferDesc.AccessFlags = GfxApiAccessFlags::GPUPrivate;
        self.TestGPUBuffer = gfxApi.CreateBuffer(bufferDesc);
    }

    tm.RegisterFrameTick_OnAnyThread(EngineFrameType::Render, DemoRendererTickPriority, DemoRendererImpl::GetData(this),
                                     QueueKind::Main);
    tm.SetFrameRate_OnMainThread(EngineFrameType::Render, 30);

    Module::Initialize(args);
}

void DemoRendererModule::Finalize()
{
    Module::Finalizing();
    if (GetUserCount() > 0)
        return;

    TimingModule&     tm = TimingModule::Get();
    GfxApiModule&     gfxApi = GfxApiModule::Get();
    WindowModule&     wm = WindowModule::Get();
    MemoryModule&     mm = MemoryModule::Get();

    DemoRendererImpl& self = *DemoRendererImpl::GetCombinePtr(this);
    gfxApi.DestroySwapChain(self.SwapChain);
    u8* range = (u8*)gfxApi.MapBuffer(self.TestUploadBuffer, 0, 1024);
    ScratchStack& stk = MemoryModule::Get().GetThreadScratchStack();
    stk.Push();
    u8* tmp = (u8*)stk.Allocate(1024);
    memcpy(tmp, range, 1024);
    for (int i = 0; i < 1024; ++i)
        CheckAlways(tmp[i] == 0x88);
    stk.Pop();

    gfxApi.DestroyBuffer(self.TestUploadBuffer);
    gfxApi.DestroyBuffer(self.TestGPUBuffer);
    self.SwapChain = (GfxApiSwapChainRef)GfxApiSwapChainRef::Null();
    for (u32 iBuffer = 0; iBuffer < BackbufferCount; ++iBuffer)
    {
        self.Backbuffers[iBuffer] = (GfxApiTextureRef)GfxApiTextureRef::Null();
    }
    tm.UnregisterFrameTick_OnAnyThread(EngineFrameType::Render, DemoRendererTickPriority);

    Module::Finalize();
    gfxApi.Release();
    tm.Release();
    wm.Release();
    mm.Release();
}

void DemoRendererModulePrivateImpl::Tick()
{
    GfxApiModule&     gfxApi = GfxApiModule::Get();

    DemoRendererImpl& self = *DemoRendererImpl::GetCombinePtr(this);
    gfxApi.CheckGpuEvents(AllQueueMask);

    {
        u32           cwidth, cheight;
        WindowModule& wm = WindowModule::Get();
        wm.GetClientAreaSize(cwidth, cheight);
        if (cwidth != self.ClientAreaSizeX || cheight != self.ClientAreaSizeY)
        {
            self.ClientAreaSizeX = cwidth;
            self.ClientAreaSizeY = cheight;
            GfxApiSwapChainDesc newDesc = self.DescSwapChain;
            newDesc.Width = cwidth;
            newDesc.Height = cheight;
            gfxApi.UpdateSwapChain(self.SwapChain, newDesc);
            gfxApi.GetBackbufferTextures(self.SwapChain, self.Backbuffers, BackbufferCount);
        }
    }

    u32               currentBuffer = gfxApi.GetCurrentBackbufferIndex(self.SwapChain);
    GfxApiRenderPass* renderPass = new GfxApiRenderPass(0);
    renderPass->RenderTargets[0].Texture = self.Backbuffers[currentBuffer];
    renderPass->RenderTargets[0].ClearValue = Vector4(1, 0, 0, 0);
    renderPass->RenderTargets[0].Action = GfxApiLoadStoreActions::Clear | GfxApiLoadStoreActions::Store;

    gfxApi.DrawRenderPass(renderPass);
    gfxApi.Present(self.SwapChain, true);
    gfxApi.ScheduleGpuEvent(GfxApiQueueType::GraphicsQueue, nullptr);
}

static Module* DemoRendererModuleCtor(const EngineInitArgMap&)
{
    return InitMemFactory<DemoRendererImpl>::New();
}

void DemoRendererModule::Destroy()
{
    InitMemFactory<DemoRendererImpl>::Delete((DemoRendererImpl*)this);
}

EXPORT_INTERNAL_MODULE(DemoRenderer, ModuleExportInfo(DemoRendererModuleCtor, true));
} // namespace Omni
