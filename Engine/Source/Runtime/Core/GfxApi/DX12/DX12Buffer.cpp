#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12Buffer.h"
#include "Runtime/Base/Memory/HandleObjectPoolImpl.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/d3dx12.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineUtils.h"


namespace Omni
{
    DX12Buffer::DX12Buffer(const GfxApiBufferDesc& desc)
        : mDesc(desc)
        , mDX12Buffer(nullptr)
    {
    }

    DX12Buffer::~DX12Buffer()
    {
    }

    const GfxApiBufferDesc DX12Buffer::GetDesc()
    {
        return mDesc;
    }

}



#endif
