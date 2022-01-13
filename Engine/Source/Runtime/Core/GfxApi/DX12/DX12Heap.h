#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiConstants.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"


struct ID3D12Heap;

namespace Omni
{
    enum class BufferHeapUsageFlag
    {
        GPUPrivate,
        UPload,
        Readback,
        //Custom,
    };
    ID3D12Heap* CreateBufferHeap(u64 size, BufferHeapUsageFlag heapUsage);
}


#endif//OMNI_WINDOWS
