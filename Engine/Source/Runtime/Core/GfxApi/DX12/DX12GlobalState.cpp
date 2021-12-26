#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/WindowsMacros.h"
#include "Runtime/Core/GfxApi/DX12/DX12Fence.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/DX12ObjectFactories.h"
#include <dxgidebug.h>


EXTERN_C const GUID DECLSPEC_SELECTANY DXGI_DEBUG_ALL = { 0xe48ae283, 0xda80, 0x490b, 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8 };


namespace Omni
{
    static constexpr bool EnableDebugLayer = OMNI_DEBUG;

    //global variable
    DX12GlobalState gDX12GlobalState;

    DX12GlobalState::DX12GlobalState() 
        : Initialized(false)
    {}

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
        CheckSucceeded(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&DXGIFactory)));
        //just use most powerful gpu
        CheckSucceeded(DXGIFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE::DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&DXGIAdaptor)));

        CheckSucceeded(D3D12CreateDevice(DXGIAdaptor, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&D3DDevice)));
        CheckGfxApi(D3DDevice->SetName(L"OmniDX12Device"));

        ID3D12InfoQueue* d3d12InfoQueue;
        CheckSucceeded(D3DDevice->QueryInterface(IID_PPV_ARGS(&d3d12InfoQueue)));
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
        CheckSucceeded(D3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&D3DGraphicsCommandQueue)));
        D3DGraphicsCommandQueue->SetName(L"OmniGraphicsQueue");
        
        /**
        * DX12 object cache
        */
        mDirectCommandListCache.Initialize(MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi), new ID3D12CommandListCacheFactory(D3DDevice), 4);
        mCommandAllocatorCache.Initialize(MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi), new ID3D12CommandAllocatorCacheFactory(D3DDevice), 4);


        /**
        * object cache
        */

        Initialized = true;
    }

    void DX12GlobalState::Finalize()
    {
        WaitGPUIdle();
        /**
        * DX12 object cache
        */
        mDirectCommandListCache.Cleanup();
        mCommandAllocatorCache.Cleanup();

        /**
        * object cache
        */

        /**
        * DX12 global obejcts
        */
        DXGIFactory = nullptr;
        DXGIAdaptor = nullptr;
        D3DGraphicsCommandQueue = nullptr;
        D3DDevice = nullptr;

        Initialized = false;
        //hack to check
#if OMNI_DEBUG
        void* checkBegin = this;
        void* checkEnd = &D3DDummyPtr;
        for (void** cp = (void**)checkBegin; cp < checkEnd; ++cp)
        {
            CheckAlways(*cp == nullptr);
        }
#endif
    }

    void DX12GlobalState::WaitGPUIdle()
    {
        auto fence = CreateFence(0);
        u64 seq = 1;
        UpdateFenceOnGPU(fence, seq, D3DGraphicsCommandQueue);
        WaitForFence(fence, seq);
        ReleaseFence(fence);
    }
}

#endif


