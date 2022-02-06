#include "Runtime/Engine/EnginePCH.h"
#include "Runtime/Engine/Render/DemoRendererModule.h"
#include "Runtime/Base/Math/Camera.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/Memory/MemoryArena.h"
#include "Runtime/Base/Memory/PageSubAllocator.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Concurrency/JobPrimitives.h"
#include "Runtime/Core/File/FileModule.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"
#include "Runtime/Core/GfxApi/GfxApiModule.h"
#include "Runtime/Core/GfxApi/GfxApiRenderPass.h"
#include "Runtime/Core/GfxApi/GfxApiBlitPass.h"
#include "Runtime/Core/GfxApi/GfxApiGraphicState.h"
#include "Runtime/Core/Platform/WindowModule.h"
#include "Runtime/Core/Platform/InputModule.h"
#include "Runtime/Core/Platform/KeyMap.h"
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

struct DemoRendererModulePrivateImpl : public ICallback, public KeyStateListener
{
public:
    DemoRendererModulePrivateImpl();
    void OnKeyEvent(KeyCode key, bool pressedf) override;
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
    GfxApiBufferRef     TestIndexBuffer;
    GfxApiBufferRef     TestVertexStream0;
    GfxApiBufferRef     TestConstantBuffer[BackbufferCount];

    std::array<GfxApiShaderRef, 2> TestShaders;
    GfxApiPSOSignatureRef          TestPSOSignature;

    GfxApiBlendStateRef        TestBlendState;
    GfxApiRasterizerStateRef   TestRasterizerState;
    GfxApiDepthStencilStateRef TestDepthStencilState;

    GfxApiGpuEventRef GpuEvent;

    TimePoint StartTime;
    Camera    Camera;
    MousePos  LastMousePos;
    bool      Moving = false;
    Vector3   TranslationDir = {};

    float RotateMouseSpeed;
    float TranslationSpeed;

    u32 ClientAreaSizeX = 0;
    u32 ClientAreaSizeY = 0;
    u32 FrameIndex = 0;

    static constexpr u32 TestCBOffset = 256;
    static constexpr u32 TestCBSize = 256;
};

struct TestShaderCB
{
    Matrix4x4 VPMatrix;
};

using DemoRendererImpl = PImplCombine<DemoRendererModule, DemoRendererModulePrivateImpl>;

/**
 * constants
 */
static constexpr u32 DemoRendererTickPriority = 100;

/**
 * definitions
 */

void DemoRendererModulePrivateImpl::OnKeyEvent(KeyCode key, bool pressed)
{
    if (key == KeyMap::MouseRight)
        Moving = pressed;

    float dir = pressed ? 1.0f : -1.0f;
    switch (key)
    {
    case KeyCode('A'):
        TranslationDir.X += -1.0f * dir;
        break;
    case KeyCode('D'):
        TranslationDir.X += +1.0f * dir;
        break;
    case KeyCode('Q'):
        TranslationDir.Y += -1.0f * dir;
        break;
    case KeyCode('E'):
        TranslationDir.Y += +1.0f * dir;
        break;
    case KeyCode('S'):
        TranslationDir.Z += -1.0f * dir;
        break;
    case KeyCode('W'):
        TranslationDir.Z += +1.0f * dir;
        break;
    }
}

DemoRendererModulePrivateImpl::DemoRendererModulePrivateImpl()
{
}

static void LoadSourceShaders(const wchar_t* paths[],
    const char*                              entryNames[],
    const GfxApiShaderStage                  stages[],
    u32                                      count,
    GfxApiShaderRef                          outShaderHandles[])
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
    InputModule& im = InputModule::Get();
    im.Retain();

    PMRAllocator gfxApiAlloc = mm.GetPMRAllocator(MemoryKind::GfxApiTmp);

    PageSubAllocator* psa = PageSubAllocator::Create(1 << 20, gfxApiAlloc, gfxApiAlloc);

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

    constexpr u32 tmpIndexBufferSize = 6 * sizeof(u16);
    constexpr u32 tmpVertexStreamSize = 4 * sizeof(Vector3);
    constexpr u32 ubVertexStreamOffset = 1024;
    { // Buffer
        auto& stk = mm.GetThreadScratchStack();
        auto  scope = stk.PushScope();
        u16*  tmpIndexBuffer = (u16*)stk.Allocate(tmpIndexBufferSize);
        tmpIndexBuffer[0] = 0;
        tmpIndexBuffer[1] = 1;
        tmpIndexBuffer[2] = 2;
        tmpIndexBuffer[3] = 1;
        tmpIndexBuffer[4] = 3;
        tmpIndexBuffer[5] = 2;

        Vector3* tmpVertexStream0 = (Vector3*)stk.Allocate(tmpVertexStreamSize);
        tmpVertexStream0[0] = Vector3(0, 0, 0);
        tmpVertexStream0[1] = Vector3(1, 0, 0);
        tmpVertexStream0[2] = Vector3(0, 1, 0);
        tmpVertexStream0[3] = Vector3(1, 1, 0);

        GfxApiBufferDesc bufferDesc;
        bufferDesc.Size = 16 * 1024;
        bufferDesc.Align = 64 * 1024;
        bufferDesc.AccessFlags = GfxApiAccessFlags::Upload;
        bufferDesc.Name = "16KUploadBuffer";
        self.TestUploadBuffer = gfxApi.CreateBuffer(bufferDesc);

        bufferDesc.Size = 16 * 1024;
        bufferDesc.AccessFlags = GfxApiAccessFlags::GPUPrivate;
        bufferDesc.Name = "16KIndexBuffer";
        self.TestIndexBuffer = gfxApi.CreateBuffer(bufferDesc);

        bufferDesc.Size = 16 * 1024;
        bufferDesc.AccessFlags = GfxApiAccessFlags::GPUPrivate;
        bufferDesc.Name = "16KVertexStream0";
        bufferDesc.UsageFlags = GfxApiResUsage::SRV;
        self.TestVertexStream0 = gfxApi.CreateBuffer(bufferDesc);

        u8* ubPtr = (u8*)gfxApi.MapBuffer(self.TestUploadBuffer, 0, tmpIndexBufferSize);
        gfxApi.MapBuffer(self.TestUploadBuffer, ubVertexStreamOffset, tmpVertexStreamSize);
        memcpy(ubPtr, tmpIndexBuffer, tmpIndexBufferSize);
        memcpy(ubPtr + ubVertexStreamOffset, tmpVertexStream0, tmpVertexStreamSize);
        gfxApi.UnmapBuffer(self.TestUploadBuffer, 0, tmpIndexBufferSize);
        gfxApi.UnmapBuffer(self.TestUploadBuffer, ubVertexStreamOffset, tmpVertexStreamSize);
    }

    { // blitpass
        GfxApiBlitPass* blitPass = new GfxApiBlitPass(psa, 2);
        blitPass->CopyBufferCmds[0] = GfxApiCopyBuffer{
            .Src = self.TestUploadBuffer,
            .SrcOffset = 0,
            .Dst = self.TestIndexBuffer,
            .DstOffset = 0,
            .Bytes = tmpIndexBufferSize,
        };
        blitPass->CopyBufferCmds[1] = GfxApiCopyBuffer{
            .Src = self.TestUploadBuffer,
            .SrcOffset = ubVertexStreamOffset,
            .Dst = self.TestVertexStream0,
            .DstOffset = 0,
            .Bytes = tmpVertexStreamSize,
        };
        gfxApi.CopyBlitPass(blitPass);
        GfxApiGpuEventRef copyDone = gfxApi.ScheduleGpuEvent(GfxApiQueueType::CopyQueue);
        gfxApi.WaitEvent(copyDone);
        gfxApi.DestroyEvent(copyDone);
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
    { // PSO
        GfxApiPSOSignatureDesc desc;
        GfxApiBindingSlot      slots[2];

        slots[0].Type = GfxApiBindingType::ConstantBuffer;
        slots[0].Space = 0;
        slots[0].BaseRegister = 0;
        slots[0].Range = 1;
        slots[0].VisibleStageMask = 1 << (u32)GfxApiShaderStage::Vertex;
        slots[0].FromBindingGroup = 0;

        slots[1].Type = GfxApiBindingType::Buffers;
        slots[1].Space = 0;
        slots[1].BaseRegister = 0;
        slots[1].Range = 1;
        slots[1].VisibleStageMask = 1 << (u32)GfxApiShaderStage::Vertex;
        slots[1].FromBindingGroup = 0;

        desc.Slots = slots;
        desc.SlotCount = 2;
        self.TestPSOSignature = gfxApi.CreatePSOSignature(desc);
    }
    {
        GfxApiBlendStateDesc desc;
        ZeroFill(desc);
        desc.Configs[0].WriteMask = 0xf;
        self.TestBlendState = gfxApi.CreateBlendState(desc);
    }
    {
        GfxApiRasterizerStateDesc desc;
        ZeroFill(desc);
        desc.FillMode = GfxApiFillMode::Solid;
        desc.CullMode = GfxApiCullMode::CullNone;
        desc.EnableRasterization = true;
        self.TestRasterizerState = gfxApi.CreateRasterizerState(desc);
    }
    {
        GfxApiDepthStencilStateDesc desc;
        ZeroFill(desc);
        desc.DepthFunc = GfxApiTestFunc::Greater;
        desc.EnableDepth = false;
        desc.EnableStencil = false;
        self.TestDepthStencilState = gfxApi.CreateDepthStencilState(desc);
    }
    {
        // binding declaration

    } { // constant buffer
        for (u32 iBuffer = 0; iBuffer < BackbufferCount; ++iBuffer)
        {
            GfxApiBufferDesc desc;
            desc.Size = 64 * 1024;
            desc.Align = 64 * 1024;
            desc.AccessFlags = GfxApiAccessFlags::Upload;
            self.TestConstantBuffer[iBuffer] = gfxApi.CreateBuffer(desc);
        }
    }
    { // Camera
        im.RegisterListener(KeyMap::MouseRight, &self);
        im.RegisterListener(KeyCode('A'), &self);
        im.RegisterListener(KeyCode('D'), &self);
        im.RegisterListener(KeyCode('W'), &self);
        im.RegisterListener(KeyCode('S'), &self);
        im.RegisterListener(KeyCode('Q'), &self);
        im.RegisterListener(KeyCode('E'), &self);

        float   aspect = float(self.ClientAreaSizeX) / self.ClientAreaSizeY;
        Vector3 eyePos(5, 0, -5);
        self.Camera = Camera(aspect, Mathf::ToRad(90), eyePos, 0, 0, 0.05f, 50.0f);
        self.RotateMouseSpeed = Mathf::PI / self.ClientAreaSizeX;
        self.TranslationSpeed = 2.0f / 30.0f;
    }

    tm.RegisterFrameTick_OnAnyThread(EngineFrameType::Render,
        DemoRendererTickPriority,
        DemoRendererImpl::GetData(this),
        QueueKind::Main);
    tm.SetFrameRate_OnMainThread(EngineFrameType::Render, 30);

    Action1<void, void*> cb(PageSubAllocator::Destroy, psa);
    gfxApi.ScheduleCompleteHandler(cb, kAllQueueMask);

    self.StartTime = std::chrono::steady_clock::now();

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
    InputModule&  im = InputModule::Get();

    DemoRendererImpl& self = *DemoRendererImpl::GetCombinePtr(this);
    gfxApi.DestroySwapChain(self.SwapChain);
    gfxApi.DestroyBuffer(self.TestUploadBuffer);
    gfxApi.DestroyBuffer(self.TestIndexBuffer);
    gfxApi.DestroyBuffer(self.TestVertexStream0);
    for (u32 iShader = 0; iShader < self.TestShaders.size(); ++iShader)
    {
        gfxApi.DestroyShader(self.TestShaders[iShader]);
    }

    gfxApi.DestroyPSOSignature(self.TestPSOSignature);
    gfxApi.DestroyBlendState(self.TestBlendState);
    gfxApi.DestroyRasterizerState(self.TestRasterizerState);
    gfxApi.DestroyDepthStencilState(self.TestDepthStencilState);

    self.SwapChain = (GfxApiSwapChainRef)GfxApiSwapChainRef::Null();
    for (u32 iBuffer = 0; iBuffer < BackbufferCount; ++iBuffer)
    {
        self.Backbuffers[iBuffer] = (GfxApiTextureRef)GfxApiTextureRef::Null();
        gfxApi.DestroyBuffer(self.TestConstantBuffer[iBuffer]);
    }
    tm.UnregisterFrameTick_OnAnyThread(EngineFrameType::Render, DemoRendererTickPriority);

    im.UnRegisterlistener(KeyMap::MouseRight, &self);
    im.UnRegisterlistener(KeyCode('A'), &self);
    im.UnRegisterlistener(KeyCode('D'), &self);
    im.UnRegisterlistener(KeyCode('W'), &self);
    im.UnRegisterlistener(KeyCode('S'), &self);
    im.UnRegisterlistener(KeyCode('Q'), &self);
    im.UnRegisterlistener(KeyCode('E'), &self);

    Module::Finalize();
    gfxApi.Release();
    tm.Release();
    wm.Release();
    mm.Release();
    fm.Release();
    im.Release();
}

void DemoRendererModulePrivateImpl::Tick()
{
    GfxApiModule&     gfxApi = GfxApiModule::Get();
    MemoryModule&     mm = MemoryModule::Get();
    InputModule&      im = InputModule::Get();
    PMRAllocator      gfxApiAlloc = mm.GetPMRAllocator(MemoryKind::GfxApiTmp);
    DemoRendererImpl& self = *DemoRendererImpl::GetCombinePtr(this);
    PageSubAllocator* psa = PageSubAllocator::Create(1 << 20, gfxApiAlloc, gfxApiAlloc);

    { // Camera logic
        // float    mouseSpeed = 1.0f;
        MousePos mousePos;
        im.GetMousePos(mousePos);
        i32 mouseDX = 0, mouseDY = 0;
        if (FrameIndex != 0 && Moving)
        {
            mouseDX = mousePos.X - self.LastMousePos.X;
            mouseDY = mousePos.Y - self.LastMousePos.Y;
            self.Camera.LocalMove(self.TranslationDir * self.TranslationSpeed);
            self.Camera.LocalRotate(mouseDY * self.RotateMouseSpeed, mouseDX * self.RotateMouseSpeed);
        }

        self.LastMousePos = mousePos;
    }

    gfxApi.CheckGpuEvents(kAllQueueMask);

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

    u32 currentBuffer = gfxApi.GetCurrentBackbufferIndex(self.SwapChain);

    { // update constant

        TestShaderCB cb;
        self.Camera.GetViewProjectionMatrix(cb.VPMatrix);

        auto cbPtr =
            (TestShaderCB*)(TestCBOffset + (u8*)gfxApi.MapBuffer(self.TestConstantBuffer[currentBuffer], 0, 0));
        memcpy(cbPtr, &cb, sizeof(TestShaderCB));
        gfxApi.UnmapBuffer(self.TestConstantBuffer[currentBuffer], TestCBOffset, sizeof(TestShaderCB));
    }

    GfxApiRenderPass* renderPass = new GfxApiRenderPass(psa, 1);
    renderPass->RenderTargets[0].Texture = self.Backbuffers[currentBuffer];
    renderPass->RenderTargets[0].ClearValue = Vector4(0, 0, 0, 0);
    renderPass->RenderTargets[0].Action = GfxApiLoadStoreActions::Clear | GfxApiLoadStoreActions::Store;

    GfxApiRenderPassStage* passStage = new GfxApiRenderPassStage(psa, 1);
    renderPass->AddStage(0, passStage);
    {
        GfxApiDrawcall& dc = passStage->Drawcalls[0];
        ZeroFill(dc);
        dc.IndexBuffer = self.TestIndexBuffer;
        dc.DrawArgs.DirectDrawArgs = GfxApiDirectDrawParams{
            .IndexCount = 6,
            .InstanceCount = 1,
            .FirstIndex = 0,
            .BaseVertex = 0,
            .BaseInstance = 0,
        };
        dc.PSOParams.Shaders[(u32)GfxApiShaderStage::Vertex] = self.TestShaders[(u32)GfxApiShaderStage::Vertex];
        dc.PSOParams.Shaders[(u32)GfxApiShaderStage::Fragment] = self.TestShaders[(u32)GfxApiShaderStage::Fragment];
        dc.PSOParams.Signature = self.TestPSOSignature;
        dc.PSOParams.BlendState = self.TestBlendState;
        dc.PSOParams.RasterizerState = self.TestRasterizerState;
        dc.PSOParams.DepthStencilState = self.TestDepthStencilState;

        GfxApiBindingGroup* bindingGroup = psa->AllocArray<GfxApiBindingGroup>(1);
        dc.BindingGroups[0] = bindingGroup;
        bindingGroup->ConstantBuffer.Buffer = self.TestConstantBuffer[currentBuffer];
        bindingGroup->ConstantBuffer.Offset = TestCBOffset;
        bindingGroup->BufferCount = 1;
        bindingGroup->Buffers = psa->AllocArray<GfxApiBufferRef>(1);
        bindingGroup->Buffers[0] = self.TestVertexStream0;
    }

    gfxApi.DrawRenderPass(renderPass);
    gfxApi.Present(self.SwapChain, true);
    gfxApi.CloseBatchDelete();
    for (u32 iQueue = 0; iQueue < u32(GfxApiQueueType::Count); ++iQueue)
    {
        GfxApiGpuEventRef gpuEvent = gfxApi.ScheduleGpuEvent(GfxApiQueueType(iQueue));
        gfxApi.DestroyEvent(gpuEvent);
    }

    Action1<void, void*> cb(PageSubAllocator::Destroy, psa);
    gfxApi.ScheduleCompleteHandler(cb, kAllQueueMask);

    ++self.FrameIndex;
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
