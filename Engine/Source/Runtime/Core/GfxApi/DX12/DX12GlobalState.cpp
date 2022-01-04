#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/WindowsMacros.h"
#include "Runtime/Core/GfxApi/DX12/DX12Fence.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/DX12ObjectFactories.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12DeleteManager.h"

#include <dxgidebug.h>


EXTERN_C const GUID DECLSPEC_SELECTANY DXGI_DEBUG_ALL = { 0xe48ae283, 0xda80, 0x490b, 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8 };


namespace Omni
{
    static constexpr bool EnableDebugLayer = OMNI_DEBUG;

    //global variable
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
            for (
                u32 iLevel = DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION; 
                iLevel < DXGI_INFO_QUEUE_MESSAGE_SEVERITY_MESSAGE; 
                ++iLevel)
            {
                CheckSucceeded(dxgiInfoQueue->SetBreakOnSeverity(
                    DXGI_DEBUG_ALL, (DXGI_INFO_QUEUE_MESSAGE_SEVERITY)iLevel, TRUE));
            }
            SafeRelease(dxgiInfoQueue);
        }
        CheckSucceeded(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&Singletons.DXGIFactory)));
        //just use most powerful gpu
        CheckSucceeded(Singletons.DXGIFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE::DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&Singletons.DXGIAdaptor)));

        CheckSucceeded(D3D12CreateDevice(Singletons.DXGIAdaptor, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&Singletons.D3DDevice)));
        CheckDX12(Singletons.D3DDevice->SetName(L"OmniDX12Device"));

        ID3D12InfoQueue* d3d12InfoQueue;
        CheckSucceeded(Singletons.D3DDevice->QueryInterface(IID_PPV_ARGS(&d3d12InfoQueue)));
        for (
            u32 iLevel = D3D12_MESSAGE_SEVERITY_CORRUPTION;
            iLevel < D3D12_MESSAGE_SEVERITY_MESSAGE;
            ++iLevel)
        {
            CheckSucceeded(d3d12InfoQueue->SetBreakOnSeverity((D3D12_MESSAGE_SEVERITY)iLevel, TRUE));
        }
        SafeRelease(d3d12InfoQueue);

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        CheckSucceeded(Singletons.D3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&Singletons.D3DQueues[(u32)GfxApiQueueType::GraphicsQueue])));
        Singletons.D3DQueues[(u32)GfxApiQueueType::GraphicsQueue]->SetName(L"OmniGraphicsQueue");
        
        /**
        * DX12 object cache
        */
        PMRAllocator gfxApiAllocator = MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi);
        DirectCommandListCache.Initialize(gfxApiAllocator, new ID3D12GraphicsCommandList4CacheFactory(Singletons.D3DDevice), 4);
        DirectCommandAllocatorCache.Initialize(gfxApiAllocator, new ID3D12CommandAllocatorCacheFactory(Singletons.D3DDevice), 4);

        /**
        * object cache
        */

        /**
        * managers
        */
        TimelineManager = OMNI_NEW(MemoryKind::GfxApi) DX12TimelineManager((ID3D12Device*)Singletons.D3DDevice);
        DeleteManager = OMNI_NEW(MemoryKind::GfxApi) DX12DeleteManager();

        Initialized = true;
    }

    void DX12GlobalState::Finalize()
    {
        DeleteManager->Flush();
        WaitGPUIdle();

        /**
        * managers
        */
        OMNI_DELETE(DeleteManager, MemoryKind::GfxApi);
        OMNI_DELETE(TimelineManager, MemoryKind::GfxApi);

        /**
        * object cache
        */


        /**
        * DX12 object cache
        */
        DirectCommandListCache.Cleanup();
        DirectCommandAllocatorCache.Cleanup();

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
        while (true)
        {
            u64 batchId;
            bool newBatchClosed = gDX12GlobalState.TimelineManager->CloseBatchAndSignalOnGPU(
                GfxApiQueueType::GraphicsQueue,
                gDX12GlobalState.Singletons.D3DQueues[(u32)GfxApiQueueType::GraphicsQueue], batchId, false);
            if (newBatchClosed)
            {//this make sure all callbacks are clean, assuming only callbacks can add new gpu commands
                gDX12GlobalState.TimelineManager->WaitBatchFinishOnGPU(GfxApiQueueType::GraphicsQueue, batchId);
                continue;
            }
            //this make sure all in flight command finishes
            gDX12GlobalState.TimelineManager->CloseBatchAndSignalOnGPU(
                GfxApiQueueType::GraphicsQueue,
                gDX12GlobalState.Singletons.D3DQueues[(u32)GfxApiQueueType::GraphicsQueue], batchId, true);
            gDX12GlobalState.TimelineManager->WaitBatchFinishOnGPU(GfxApiQueueType::GraphicsQueue, batchId);
            break;
        }
    }
}

#endif


