#include "Runtime/Engine/EnginePCH.h"
#include "Runtime/Engine/Render/DemoRendererModule.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/Memory/MemoryArena.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Concurrency/JobPrimitives.h"
#include "Runtime/Core/File/FileModule.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"
#include "Runtime/Core/GfxApi/GfxApiModule.h"
#include "Runtime/Core/GfxApi/GfxApiRenderPass.h"
#include "Runtime/Core/GfxApi/GfxApiBlitPass.h"
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
    DispatchWorkItem*              TickRegistry = nullptr;
    GfxApiSwapChainDesc            DescSwapChain;
    GfxApiSwapChainRef             SwapChain = {};
    GfxApiTextureRef               Backbuffers[BackbufferCount];
    GfxApiBufferRef                TestUploadBuffer;
    GfxApiBufferRef                TestGPUBuffer;
    std::array<GfxApiShaderRef, 2> TestShaders;
    GfxApiGpuEventRef              GpuEvent;
    u32                            ClientAreaSizeX = 0;
    u32                            ClientAreaSizeY = 0;
    u32                            FrameIndex = 0;
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

static void LoadSourceShaders(const wchar_t*          paths[],
                              const char*             entryNames[],
                              const GfxApiShaderStage stages[],
                              u32                     count,
                              GfxApiShaderRef         outShaderHandles[])
{
    MemoryModule& mm = MemoryModule::Get();
    FileModule&   fm = FileModule::Get();
    GfxApiModule& gfxApi = GfxApiModule::Get();

    PMRUTF16String tpath(mm.GetPMRAllocator(MemoryKind::UserDefault));
    PMRVector<u8>  tdata(mm.GetPMRAllocator(MemoryKind::UserDefault));

    for (u32 iShader = 0; iShader < count; ++iShader)
    {
        fm.GetPath(tpath, PredefinedPath::ProjectRoot, paths[iShader]);
        fm.ReadFileContent(tpath, tdata);

        GfxApiShaderDesc shaderDesc;
        shaderDesc.EntryName = shaderDesc.Name = entryNames[iShader];

        shaderDesc.Source = std::string_view((const char*)tdata.data(), tdata.size());
        shaderDesc.Stage = stages[iShader];

        outShaderHandles[iShader] = gfxApi.CreateShader(shaderDesc);

        tpath.clear();
        tdata.clear();
    }
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
    FileModule& fm = FileModule::Get();
    fm.Retain();

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

    constexpr u32 tmpBufferSize = 6 * sizeof(u16);
    { // Buffer
        auto& stk = mm.GetThreadScratchStack();
        auto  scope = stk.PushScope();
        u16*  tmpBuffer = (u16*)stk.Allocate(tmpBufferSize);
        tmpBuffer[0] = 0;
        tmpBuffer[1] = 1;
        tmpBuffer[2] = 2;
        tmpBuffer[3] = 1;
        tmpBuffer[4] = 3;
        tmpBuffer[5] = 2;

        GfxApiBufferDesc bufferDesc;
        bufferDesc.Size = 16 * 1024;
        bufferDesc.AccessFlags = GfxApiAccessFlags::Upload;
        bufferDesc.Name = "16KUploadBuffer";
        self.TestUploadBuffer = gfxApi.CreateBuffer(bufferDesc);
        u8* range = (u8*)gfxApi.MapBuffer(self.TestUploadBuffer, 0, tmpBufferSize);
        memcpy(range, tmpBuffer, tmpBufferSize);
        gfxApi.UnmapBuffer(self.TestUploadBuffer, 0, tmpBufferSize);

        bufferDesc.Size = 32 * 1024;
        bufferDesc.AccessFlags = GfxApiAccessFlags::GPUPrivate;
        bufferDesc.Name = "32KGPUBuffer";
        self.TestGPUBuffer = gfxApi.CreateBuffer(bufferDesc);
    }
    
    { // blitpass
        GfxApiBlitPass* blitPass = new GfxApiBlitPass(1);
        blitPass->CopyBufferCmds[0] = GfxApiCopyBuffer{
            .Src = self.TestUploadBuffer,
            .SrcOffset = 0,
            .Dst = self.TestGPUBuffer,
            .DstOffset = 0,
            .Bytes = tmpBufferSize,
        };
        gfxApi.CopyBlitPass(blitPass);
        //GfxApiGpuEventRef copyDone = gfxApi.ScheduleGpuEvent(GfxApiQueueType::CopyQueue);
        //gfxApi.WaitEvent(copyDone);
        //gfxApi.DestroyEvent(copyDone);
    }
    { // Shader
        // self.TestShaderVS = gfxApi.CreateShader(shaderDesc);
        const wchar_t* paths[] = {
            L"Assets/Shader/SimpleTest_VS.hlsl",
            L"Assets/Shader/SimpleTest_FS.hlsl",
        };
        const char* entryNames[] = {
            "VSMain", // VS
            "FSMain", // PS
        };

        GfxApiShaderStage stages[] = {
            GfxApiShaderStage::Vertex,
            GfxApiShaderStage::Fragment,
        };

        LoadSourceShaders(paths, entryNames, stages, 2, &self.TestShaders[0]);
    }

    tm.RegisterFrameTick_OnAnyThread(EngineFrameType::Render,
                                     DemoRendererTickPriority,
                                     DemoRendererImpl::GetData(this),
                                     QueueKind::Main);
    tm.SetFrameRate_OnMainThread(EngineFrameType::Render, 30);

    Module::Initialize(args);
}

void DemoRendererModule::Finalize()
{
    Module::Finalizing();
    if (GetUserCount() > 0)
        return;

    TimingModule& tm = TimingModule::Get();
    GfxApiModule& gfxApi = GfxApiModule::Get();
    WindowModule& wm = WindowModule::Get();
    MemoryModule& mm = MemoryModule::Get();
    FileModule&   fm = FileModule::Get();

    DemoRendererImpl& self = *DemoRendererImpl::GetCombinePtr(this);
    gfxApi.DestroySwapChain(self.SwapChain);
    gfxApi.DestroyBuffer(self.TestUploadBuffer);
    gfxApi.DestroyBuffer(self.TestGPUBuffer);
    for (u32 iShader = 0; iShader < self.TestShaders.size(); ++iShader)
    {
        gfxApi.DestroyShader(self.TestShaders[iShader]);
    }
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
    fm.Release();
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
    GfxApiGpuEventRef gpuEvent = gfxApi.ScheduleGpuEvent(GfxApiQueueType::GraphicsQueue);
    gfxApi.DestroyEvent(gpuEvent);
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
