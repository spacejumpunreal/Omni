#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12Buffer.h"
#include "Runtime/Base/Memory/HandleObjectPoolImpl.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/DX12BufferManager.h"
#include "Runtime/Core/GfxApi/DX12/d3dx12.h"


namespace Omni
{
    DX12Buffer::DX12Buffer(const GfxApiBufferDesc& desc)
        : mDesc(desc)
    {
        gDX12GlobalState.BufferManager->AllocBuffer(desc, mDX12Buffer, mAllocHandle);
    }

    DX12Buffer::~DX12Buffer()
    {
        //TODO: add deferred delete
        NotImplemented();
    }

    const GfxApiBufferDesc DX12Buffer::GetDesc()
    {
        return mDesc;
    }

}



#endif
