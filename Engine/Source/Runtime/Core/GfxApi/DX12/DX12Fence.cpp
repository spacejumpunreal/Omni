#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12Fence.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include <d3d12.h>

namespace Omni
{
	ID3D12Fence* CreateFence(u64 initValue, ID3D12Device* dev)
	{
		ID3D12Fence* ret = nullptr;
        dev = dev == nullptr ? ((ID3D12Device*)gDX12GlobalState.Singletons.D3DDevice) : dev;
		CheckDX12(dev->CreateFence(initValue, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&ret)));
		return ret;
	}
	void UpdateFenceOnGPU(ID3D12Fence* fence, u64 newValue, ID3D12CommandQueue* queue)
	{
		CheckDX12(queue->Signal(fence, newValue));
	}
	void ReleaseFence(ID3D12Fence* fence)
	{
		fence->Release();
	}
	void WaitForFence(ID3D12Fence* fence, u64 waitValue)
	{
        if (fence->GetCompletedValue() >= waitValue)
            return;
		HANDLE winHandle = ::CreateEvent(nullptr, FALSE, FALSE, L"WaitForFenceInPlace");
		if (winHandle != nullptr)
		{
			CheckDX12(fence->SetEventOnCompletion(waitValue, winHandle));
			while (true)
			{
				if (::WaitForSingleObject(winHandle, INFINITE) == WAIT_OBJECT_0)
				{
					CloseHandle(winHandle);
					return;
				}
			}

		}
		else
		{
			CheckAlways(false, "CreateEvent failed");
		}
	}
}

#endif//OMNI_WINDOWS
