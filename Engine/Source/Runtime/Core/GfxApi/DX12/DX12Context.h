#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/Platform/OSUtils_Windows.h"
#include <d3d12.h>
#include <dxgi1_6.h>


namespace Omni
{
	struct DX12Context
	{
	public:
		DX12Context();
		void Initialize();
		void Finalize();
	public:
		ComPtr<IDXGIFactory7>				DXGIFactory;
		ComPtr<IDXGIAdapter1>				DXGIAdaptor;
		ComPtr<ID3D12Device>				D3DDevice;
		ComPtr<ID3D12CommandQueue>			D3DCommandQueue;
		bool								Initialized;
	};
	
	extern DX12Context gDX12Context;
}

#endif//OMNI_WINDOWS