#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/Platform/OSUtils_Windows.h"
#include "Runtime/Base/Memory/ObjectCache.h"
#include <d3d12.h>
#include <dxgi1_6.h>


namespace Omni
{
    //forward decls

	struct DX12GlobalState
	{
	public:
		DX12GlobalState();
		void Initialize();
		void Finalize();
		void WaitGPUIdle();
	public://DX12 global objects
		ComPtr<IDXGIFactory7>				            DXGIFactory;
		ComPtr<IDXGIAdapter1>				            DXGIAdaptor;
		ComPtr<ID3D12Device>				            D3DDevice;
		ComPtr<ID3D12CommandQueue>			            D3DGraphicsCommandQueue;
        ComPtr<ID3D12Resource>			                D3DDummyPtr; //keep this the last one
    public://DX12 object pools
        ObjectCache<ID3D12GraphicsCommandList4>         DirectCommandListCache;
        ObjectCache<ID3D12CommandAllocator>             DirectCommandAllocatorCache;
        //we can have a ID3D12Fence cache here
    public://object pools

    public://state flags
        bool								            Initialized;
    public://managers
        class DX12TimelineManager*                      TimelineManager;
        class DX12DeleteManager*                        DeleteManager;
	};
	
	extern DX12GlobalState gDX12GlobalState;
}

#endif//OMNI_WINDOWS
