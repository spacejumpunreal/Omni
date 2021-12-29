#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS

struct ID3D12Fence;
struct ID3D12Device;
namespace Omni
{
	ID3D12Fence* CreateFence(u64 initValue, ID3D12Device* dev = nullptr);
	void UpdateFenceOnGPU(ID3D12Fence* fence, u64 newValue, ID3D12CommandQueue* queue);
    void ReleaseFence(ID3D12Fence* fence);
    void WaitForFence(ID3D12Fence* fence, u64 waitValue);
}

#endif//OMNI_WINDOWS
