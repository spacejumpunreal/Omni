#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Base/Memory/HandleObjectPoolImpl.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/WindowsMacros.h"
#include "Runtime/Core/GfxApi/DX12/DX12Fence.h"
#include "Runtime/Core/GfxApi/DX12/DX12Texture.h"
#include "Runtime/Core/GfxApi/DX12/DX12Buffer.h"
#include "Runtime/Core/GfxApi/DX12/DX12SwapChain.h"
#include "Runtime/Core/GfxApi/DX12/DX12Event.h"
#include "Runtime/Core/GfxApi/DX12/DX12Shader.h"
#include "Runtime/Core/GfxApi/DX12/DX12GraphicsState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/DX12ObjectFactories.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12DeleteManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12BufferManager.h"
#include "Runtime/Core/GfxApi/DX12/DXCWrapper.h"
#include "Runtime/Core/GfxApi/DX12/DX12PSOManager.h"
#include <d3d12.h>

#include <dxgidebug.h>

EXTERN_C const GUID DECLSPEC_SELECTANY DXGI_DEBUG_ALL = {
    0xe48ae283, 0xda80, 0x490b, 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8};

namespace Omni
{
// decls
void CheckSupportedFeatures(ID3D12Device* device);

static constexpr bool EnableDebugLayer = OMNI_DEBUG;

// global variable
DX12GlobalState gDX12GlobalState;

DX12Singletons::DX12Singletons()
{
    memset(this, 0, sizeof(*this));
}
void DX12Singletons::Finalize()
{
    IUnknown** ptr = (IUnknown**)this;
    for (u32 iPtr = 0; iPtr < sizeof(*this) / sizeof(void*); ++iPtr)
    {
        if (ptr[iPtr] != nullptr)
            SafeRelease(ptr[iPtr]);
    }
}
DX12GlobalState::DX12GlobalState()
    : Initialized(false)
    , TimelineManager(nullptr)
    , DeleteManager(nullptr)
    , BufferManager(nullptr)
    , DXCInstance(nullptr)
    , PSOManager(nullptr)
{
}

void DX12GlobalState::Initialize()
{
    /**
     * DX12 global obejcts
     */
    UINT dxgiFactoryFlags = 0;
    if constexpr (EnableDebugLayer)
    {
        // Enable the debug layer (requires the Graphics Tools "optional feature").
        // NOTE: Enabling the debug layer after device creation will invalidate the active device.
        ID3D12Debug* debugController;
        CheckSucceeded(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
        debugController->EnableDebugLayer();
        SafeRelease(debugController);

        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;

        IDXGIInfoQueue* dxgiInfoQueue;
        CheckSucceeded(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiInfoQueue)));
        for (u32 iLevel = DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION;
             iLevel < DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE;
             ++iLevel)
        {
            CheckSucceeded(
                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, (DXGI_INFO_QUEUE_MESSAGE_SEVERITY)iLevel, TRUE));
        }
        SafeRelease(dxgiInfoQueue);
    }
    CheckSucceeded(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&Singletons.DXGIFactory)));
    // just use most powerful gpu
    CheckSucceeded(Singletons.DXGIFactory->EnumAdapterByGpuPreference(0,
        DXGI_GPU_PREFERENCE::DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
        IID_PPV_ARGS(&Singletons.DXGIAdaptor)));

    CheckSucceeded(
        D3D12CreateDevice(Singletons.DXGIAdaptor, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&Singletons.D3DDevice)));
    CheckDX12(Singletons.D3DDevice->SetName(L"OmniDX12Device"));

    ID3D12InfoQueue* d3d12InfoQueue;
    CheckSucceeded(Singletons.D3DDevice->QueryInterface(IID_PPV_ARGS(&d3d12InfoQueue)));
    for (u32 iLevel = D3D12_MESSAGE_SEVERITY_CORRUPTION; iLevel < D3D12_MESSAGE_SEVERITY_MESSAGE; ++iLevel)
    {
        CheckSucceeded(d3d12InfoQueue->SetBreakOnSeverity((D3D12_MESSAGE_SEVERITY)iLevel, TRUE));
    }
    SafeRelease(d3d12InfoQueue);

    { // Queues
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        CheckSucceeded(Singletons.D3DDevice->CreateCommandQueue(&queueDesc,
            IID_PPV_ARGS(&Singletons.D3DQueues[(u32)GfxApiQueueType::GraphicsQueue])));
        Singletons.D3DQueues[(u32)GfxApiQueueType::GraphicsQueue]->SetName(L"OmniGraphicsQueue");

        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
        CheckSucceeded(Singletons.D3DDevice->CreateCommandQueue(&queueDesc,
            IID_PPV_ARGS(&Singletons.D3DQueues[(u32)GfxApiQueueType::ComputeQueue])));
        Singletons.D3DQueues[(u32)GfxApiQueueType::ComputeQueue]->SetName(L"OmniComputeQueue");

        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
        CheckSucceeded(Singletons.D3DDevice->CreateCommandQueue(&queueDesc,
            IID_PPV_ARGS(&Singletons.D3DQueues[(u32)GfxApiQueueType::CopyQueue])));
        Singletons.D3DQueues[(u32)GfxApiQueueType::CopyQueue]->SetName(L"OmniCopyQueue");
    }

    /**
     * DX12 object cache
     */
    PMRAllocator gfxApiAllocator = MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi);
    for (u32 iQueueType = (u32)GfxApiQueueType::GraphicsQueue; iQueueType < (u32)GfxApiQueueType::Count; ++iQueueType)
    {
        D3D12_COMMAND_LIST_TYPE cmdType = QueueTypeToCmdType((GfxApiQueueType)iQueueType);
        CommandListCache[iQueueType].Initialize(gfxApiAllocator,
            new ID3D12GraphicsCommandList4CacheFactory(Singletons.D3DDevice, cmdType),
            4);
        CommandAllocatorCache[iQueueType].Initialize(gfxApiAllocator,
            new ID3D12CommandAllocatorCacheFactory(Singletons.D3DDevice, cmdType),
            4);
    }

    /**
     * object cache
     */
    DX12SwapChainPool.Initialize(gfxApiAllocator, 2);
    DX12BufferPool.Initialize(gfxApiAllocator, 64);
    DX12TexturePool.Initialize(gfxApiAllocator, 4);
    DX12ShaderPool.Initialize(gfxApiAllocator, 16);
    DX12GpuEventPool.Initialize(gfxApiAllocator, 16);
    DX12PSOSignaturePool.Initialize(gfxApiAllocator, 16);
    DX12BlendStatePool.Initialize(gfxApiAllocator, 16);
    DX12RasterizerStatePool.Initialize(gfxApiAllocator, 16);
    DX12DepthStencilStatePool.Initialize(gfxApiAllocator, 16);

    /**
     * managers
     */
    TimelineManager = DX12TimelineManager::Create((ID3D12Device*)Singletons.D3DDevice);
    DeleteManager = DX12DeleteManager::Create();
    BufferManager = DX12BufferManager::Create();
    DXCInstance = DXCWrapper::Create();
    PSOManager = DX12PSOManager::Create();

    CheckSupportedFeatures(Singletons.D3DDevice);

    Initialized = true;
}

void DX12GlobalState::Finalize()
{
#define DESTROY_MANAGER(Manager)                                                                                       \
    Manager->Destroy();                                                                                                \
    Manager = nullptr

    /**
     * managers
     */
    // PSOManager rely on Finalize to cleanup, must be done before DeleteManager::Destroy()
    DESTROY_MANAGER(PSOManager);
    DESTROY_MANAGER(DXCInstance);

    DeleteManager->Flush();
    WaitGPUIdle();

    DESTROY_MANAGER(DeleteManager);
    DESTROY_MANAGER(TimelineManager);
    DESTROY_MANAGER(BufferManager);

#undef DESTROY_MANAGER
    /**
     * object cache
     */
    DX12SwapChainPool.Finalize();
    DX12BufferPool.Finalize();
    DX12TexturePool.Finalize();
    DX12ShaderPool.Finalize();
    DX12GpuEventPool.Finalize();
    DX12PSOSignaturePool.Finalize();
    DX12BlendStatePool.Finalize();
    DX12RasterizerStatePool.Finalize();
    DX12DepthStencilStatePool.Finalize();

    /**
     * DX12 object cache
     */
    for (u32 iCmdType = (u32)GfxApiQueueType::GraphicsQueue; iCmdType < (u32)GfxApiQueueType::Count; ++iCmdType)
    {
        CommandListCache[iCmdType].Cleanup();
        CommandAllocatorCache[iCmdType].Cleanup();
    }
    /**
     * object cache
     */

    /**
     * DX12 global obejcts
     */
    Singletons.Finalize();

    Initialized = false;
}

void DX12GlobalState::WaitGPUIdle()
{
    bool silent;
    do
    {
        silent = true;
        for (u32 iQueueType = 0; iQueueType < (u32)GfxApiQueueType::Count; ++iQueueType)
        {
            auto queueType = (GfxApiQueueType)iQueueType;
            u64  batchId;
            bool newBatchClosed = gDX12GlobalState.TimelineManager->CloseBatchAndSignalOnGPU(queueType,
                gDX12GlobalState.Singletons.D3DQueues[iQueueType],
                batchId,
                false);
            if (newBatchClosed)
            { // this make sure all callbacks are clean, assuming only callbacks can add new gpu commands
                gDX12GlobalState.TimelineManager->WaitBatchFinishOnGPU(queueType, batchId);
                silent = false;
                continue;
            }
            // this make sure all in flight command finishes
            gDX12GlobalState.TimelineManager->CloseBatchAndSignalOnGPU(queueType,
                gDX12GlobalState.Singletons.D3DQueues[(u32)iQueueType],
                batchId,
                true);
            gDX12GlobalState.TimelineManager->WaitBatchFinishOnGPU(queueType, batchId);
        }
    } while (!silent);
}

void CheckSupportedFeatures(ID3D12Device* device)
{
    // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_heap_tier
    D3D12_FEATURE_DATA_D3D12_OPTIONS featureOptions{};
    CheckSucceeded(device->CheckFeatureSupport(D3D12_FEATURE::D3D12_FEATURE_D3D12_OPTIONS,
        &featureOptions,
        sizeof(featureOptions)));

    // check SM6.0 support
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = {D3D_SHADER_MODEL_6_0};
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel))) ||
        (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0))
    {
        CheckAlways(false, "Shader Model 6.0 is not supported!");
    }
}
} // namespace Omni

#endif
