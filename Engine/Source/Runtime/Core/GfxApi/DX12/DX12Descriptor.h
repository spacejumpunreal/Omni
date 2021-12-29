#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12Basics.h"
#include <d3d12.h>

namespace Omni
{
    constexpr D3D12_CPU_DESCRIPTOR_HANDLE NullCPUDescriptorHandle = {};
    FORCEINLINE D3D12_CPU_DESCRIPTOR_HANDLE ToCPUDescriptorHandle(DX12Descriptor desc)
    {
        return D3D12_CPU_DESCRIPTOR_HANDLE{ .ptr = desc };
    }
}


#endif//OMNI_WINDOWS
