#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12BufferManager.h"
#include "Runtime/Base/Math/SepcialFunctions.h"
#include "Runtime/Base/Memory/ExternalAllocatorImpl.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/DX12Heap.h"
#include "Runtime/Core/GfxApi/DX12/d3dx12.h"

namespace Omni
{
// declarations
struct DX12BufferMemProvider final : public IExternalMemProvider
{
public:
    void Initialize(GfxApiAccessFlags accessFlag, u64 minBlockSize);
    void Finalize();

public:
    void Map(u64 size, u64 align, ExtenralAllocationBlockId& outBlockId) override;
    void Unmap(ExtenralAllocationBlockId blockId) override;
    u64  SuggestNewBlockSize(u64 reqSize) override;

public:
    D3D12_HEAP_DESC HeapDesc = {};
    u64             MinBlockSize = (u64)-1;

#if OMNI_DEBUG
private:
    u32 MapCount = 0;
#endif
};

using DX12BufferFirstFitAllocator = BestFitAllocator<DX12BufferMemProvider>;

struct DX12BufferMangerPrivateData
{
public:
    DX12BufferFirstFitAllocator mAllocator[(u32)D3D12_HEAP_TYPE_READBACK + 1];
};

using DX12BufferManagerImpl = PImplCombine<DX12BufferManager, DX12BufferMangerPrivateData>;

/**
 * DX12BufferMemProvider
 */
void DX12BufferMemProvider::Initialize(GfxApiAccessFlags accessFlag, u64 minBlockSize)
{
    MinBlockSize = minBlockSize;
    BuildHeapDesc(HeapDesc, accessFlag);
    MapCount = 0;
}

void DX12BufferMemProvider::Finalize()
{
    CheckDebug(MapCount == 0, "not all buffer heap destroy, there's some leak");
}

void DX12BufferMemProvider::Map(u64 size, u64 align, ExtenralAllocationBlockId& outBlockId)
{
    CheckAlways(align <= 64 * 1024); // 64k is the only support alignment
    ID3D12Heap* heap;
    HeapDesc.SizeInBytes = size;
    CheckDX12(gDX12GlobalState.Singletons.D3DDevice->CreateHeap(&HeapDesc, IID_PPV_ARGS(&heap)));
    outBlockId = (ExtenralAllocationBlockId)heap;
    ++MapCount;
}

void DX12BufferMemProvider::Unmap(ExtenralAllocationBlockId blockId)
{
    ((ID3D12Heap*)blockId)->Release();
    --MapCount;
}

u64 DX12BufferMemProvider::SuggestNewBlockSize(u64 reqSize)
{
    return Mathf::Max(reqSize, MinBlockSize);
}

/**
 * DX12BufferManager
 */
DX12BufferManager* DX12BufferManager::Create()
{
    DX12BufferManagerImpl* self = OMNI_NEW(MemoryKind::GfxApi) DX12BufferManagerImpl();
    self->mAllocator[D3D12_HEAP_TYPE_DEFAULT].mProvider.Initialize(
        GfxApiAccessFlags::GPURead | GfxApiAccessFlags::GPUWrite, 32 * 1024 * 1024);
    self->mAllocator[D3D12_HEAP_TYPE_UPLOAD].mProvider.Initialize(
        GfxApiAccessFlags::GPURead | GfxApiAccessFlags::CPUWrite, 64 * 1024 * 1024);
    self->mAllocator[D3D12_HEAP_TYPE_READBACK].mProvider.Initialize(
        GfxApiAccessFlags::CPURead | GfxApiAccessFlags::GPUWrite, 1024 * 1024);
    return self;
}

void DX12BufferManager::Destroy()
{
    DX12BufferManagerImpl* self = DX12BufferManagerImpl::GetCombinePtr(this);
    self->mAllocator[D3D12_HEAP_TYPE_DEFAULT].Cleanup();
    self->mAllocator[D3D12_HEAP_TYPE_DEFAULT].mProvider.Finalize();
    self->mAllocator[D3D12_HEAP_TYPE_UPLOAD].Cleanup();
    self->mAllocator[D3D12_HEAP_TYPE_UPLOAD].mProvider.Finalize();
    self->mAllocator[D3D12_HEAP_TYPE_READBACK].Cleanup();
    self->mAllocator[D3D12_HEAP_TYPE_READBACK].mProvider.Finalize();
    OMNI_DELETE(self, MemoryKind::GfxApi);
}

void DX12BufferManager::AllocBuffer(const GfxApiBufferDesc& desc, ID3D12Resource*& dx12Buffer,
                                    ExternalAllocationHandle& allocHandle)
{
    DX12BufferManagerImpl* self = DX12BufferManagerImpl::GetCombinePtr(this);
    D3D12_HEAP_TYPE        ht;
    ToD3D12HeapType(ht, desc.AccessFlags);
    ExternalAllocation ealloc = self->mAllocator[ht].Alloc(desc.Size, desc.Align);
    D3D12_RESOURCE_DESC rDesc;
    rDesc = CD3DX12_RESOURCE_DESC::Buffer(ealloc.Size, )?????
    gDX12GlobalState.Singletons.D3DDevice->CreatePlacedResource((ID3D12Heap*)ealloc.BlockId, ealloc.Start, &rDesc,
                                                                D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON,
                                                                nullptr, IID_PPV_ARGS(&dx12Buffer));
    allocHandle = ealloc.Handle;
}

} // namespace Omni

#endif // OMNI_WINDOWS
