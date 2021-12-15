#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12Context.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Core/Platform/WindowsMacros.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"


namespace Omni
{
    static constexpr bool EnableDebugLayer = OMNI_DEBUG;

    //global variable
    DX12Context gDX12Context;

    DX12Context::DX12Context() 
        :Initialized(false) 
    {}

	void DX12Context::Initialize()
	{
        UINT dxgiFactoryFlags = 0;
        if constexpr (EnableDebugLayer)
        {
            // Enable the debug layer (requires the Graphics Tools "optional feature").
            // NOTE: Enabling the debug layer after device creation will invalidate the active device.
            {
                ID3D12Debug* debugController;
                if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
                {
                    debugController->EnableDebugLayer();

                    // Enable additional debug layers.
                    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
                    SafeRelease(debugController);
                }
            }
        }
        CheckSucceeded(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&DXGIFactory)));
        //just use most powerful gpu
        CheckSucceeded(DXGIFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE::DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&DXGIAdaptor)));

        CheckSucceeded(D3D12CreateDevice(DXGIAdaptor, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&D3DDevice)));
        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        CheckSucceeded(D3DDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&D3DCommandQueue)));

        Initialized = true;
	}

    void DX12Context::Finalize()
    {
        DXGIFactory = nullptr;
        DXGIAdaptor = nullptr;
        D3DDevice = nullptr;
        D3DCommandQueue = nullptr;

        Initialized = false;
        //hack to check
#if OMNI_DEBUG
        void* checkBegin = this;
        void* checkEnd = &Initialized;
        for (void** cp = (void**)checkBegin; cp < checkEnd; ++cp)
        {
            CheckAlways(*cp == nullptr);
        }
#endif
    }
}

#endif


