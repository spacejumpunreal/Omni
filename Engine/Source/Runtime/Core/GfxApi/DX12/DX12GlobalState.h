#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/Platform/OSUtils_Windows.h"
#include "Runtime/Base/Memory/ObjectCache.h"
#include "Runtime/Core/GfxApi/DX12/DX12CommandContext.h"
#include <d3d12.h>
#include <dxgi1_6.h>


namespace Omni
{
	struct DX12GlobalState
	{
	public:
		DX12GlobalState();
		void Initialize();
		void Finalize();
		void WaitGPUIdle();
	public://DX12 global objects
		ComPtr<IDXGIFactory7>				DXGIFactory;
		ComPtr<IDXGIAdapter1>				DXGIAdaptor;
		ComPtr<ID3D12Device>				D3DDevice;
		ComPtr<ID3D12CommandQueue>			D3DGraphicsCommandQueue;
    public://DX12 object pools

    public://object pools
    public://state flags
		bool								Initialized;
	};
	
	extern DX12GlobalState gDX12GlobalState;
}

#endif//OMNI_WINDOWS
