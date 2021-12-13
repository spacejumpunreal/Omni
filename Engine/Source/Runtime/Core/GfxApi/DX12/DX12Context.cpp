#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12Context.h"
#include "Runtime/Base/Misc/AssertUtils.h"

namespace Omni
{
    static constexpr bool EnableDebugLayer = true;

	DX12Context::DX12Context()
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
                }
            }
        }
        CheckSucceeded(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(Factory)));
	}
}

#endif


