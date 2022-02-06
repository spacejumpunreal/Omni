#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12Buffer.h"
#include "Runtime/Base/Memory/HandleObjectPoolImpl.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/DX12BufferManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12DeleteManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12DescriptorManager.h"
#include "Runtime/Core/GfxApi/DX12/d3dx12.h"

namespace Omni
{


DX12Buffer::DX12Buffer(const GfxApiBufferDesc& desc)
    : DX12Resource(CalcInitialStateFromAccessFlag(desc.AccessFlags))
    , mDesc(desc)
{
    gDX12GlobalState.BufferManager->AllocBuffer(desc, mDX12Resource, mMemAlloc);
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    ID3D12DescriptorHeap*       heap;
    gDX12GlobalState.DescriptorManager
        ->AllocRange(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, false, 1, mDescAlloc, cpuHandle, gpuHandle, heap);
    mSRVDescriptor = cpuHandle.ptr;
    Setname(mDesc.Name);
}

DX12Buffer::~DX12Buffer()
{
    gDX12GlobalState.DescriptorManager->FreeRange(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, false, mDescAlloc);
    gDX12GlobalState.BufferManager->FreeBuffer(mDesc.AccessFlags, mDX12Resource, mMemAlloc);
    mDX12Resource = nullptr;
    mSRVDescriptor = NullDX12Descriptor;
}

const GfxApiBufferDesc DX12Buffer::GetDesc() const
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
    CheckDX12(mDX12Resource->Map(0, mapSize == 0 ? nullptr : &range, &ptr));
    return ptr;
}

void DX12Buffer::Unmap(u32 beginOffset, u32 mapSize)
{
    D3D12_RANGE range;
    range.Begin = beginOffset;
    range.End = beginOffset + mapSize;
    mDX12Resource->Unmap(0, &range);
}

DX12Descriptor DX12Buffer::GetSRVDescriptor() const
{
    return mSRVDescriptor;
}

} // namespace Omni

#endif
