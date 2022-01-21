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
    : DX12Resource(CalcInitialStateFromAccessFlag(desc.AccessFlags))
    , mDesc(desc)
{
    gDX12GlobalState.BufferManager->AllocBuffer(desc, mDX12Resource, mAllocHandle);
}

DX12Buffer::~DX12Buffer()
{
    gDX12GlobalState.BufferManager->FreeBuffer(mDesc.AccessFlags, mDX12Resource, mAllocHandle);
    mDX12Resource = nullptr;
}

const GfxApiBufferDesc DX12Buffer::GetDesc()
{
    return mDesc;
}

void* DX12Buffer::Map(u32 beginOffset, u32 mapSize)
{
    CheckDebug(mDesc.AccessFlags != GfxApiAccessFlags::GPUPrivate);
    D3D12_RANGE range;
    range.Begin = beginOffset;
    range.End = beginOffset + mapSize;
    void* ptr;
    CheckDX12(mDX12Resource->Map(0, &range, &ptr));
    return ptr;
}

void DX12Buffer::Unmap(u32 beginOffset, u32 mapSize)
{
    D3D12_RANGE range;
    range.Begin = beginOffset;
    range.End = beginOffset + mapSize;
    mDX12Resource->Unmap(0, &range);
}


} // namespace Omni

#endif
