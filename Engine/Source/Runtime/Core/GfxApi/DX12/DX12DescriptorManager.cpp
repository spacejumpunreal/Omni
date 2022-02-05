#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/GfxApi/DX12/DX12DescriptorManager.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/Memory/ExternalAllocatorImpl.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/d3dx12.h"
#if OMNI_WINDOWS

namespace Omni
{
/*
 * declaration
 */
struct DX12DescriptorMemProvider final : public IExternalMemProvider
{
    // https://docs.microsoft.com/en-us/windows/win32/direct3d12/shader-visible-descriptor-heaps
    // write-combine memory hardware limit is usually 96M, 32Byte per descriptor
public:
    void Initialize(D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible);
    void Finalize();

public:
    void         Map(u64 size, u64 align, ExtenralAllocationBlockId& outBlockId) override;
    void         Unmap(ExtenralAllocationBlockId blockId) override;
    u64          SuggestNewBlockSize(u64 reqSize) override;
    PMRAllocator GetCPUAllocator() override;

private:
    D3D12_DESCRIPTOR_HEAP_TYPE mType;
    bool                       mShaderVisible;

public:
#if OMNI_DEBUG
private:
    u32 mUsedCount = 0;
#endif
};

using DX12PersistentDescAllocator = BestFitAllocator<DX12DescriptorMemProvider>;
using DX12TmpDescAllocator = BestFitAllocator<DX12DescriptorMemProvider>;

/*
 * constants
 */
static constexpr u32 kHeapTypeCount = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;

struct DX12DescriptorManagerPrivateData
{
public:
    DX12PersistentDescAllocator PersistentAllocs[kHeapTypeCount];
    DX12TmpDescAllocator        TmpAllocs[kHeapTypeCount];
    u32                         HeapStepSize[(u32)D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
};

/*
 * globals
 */
u32 DX12DescriptorManager::mHeapStepSize[4];

using DX12DescriptorManagerImpl = PImplCombine<DX12DescriptorManager, DX12DescriptorManagerPrivateData>;

/*
 * definition
 */

// DX12DescriptorManagerPrivateData

// DX12DescriptorManager
DX12DescriptorManager* DX12DescriptorManager::Create()
{
    DX12DescriptorManagerImpl* self = OMNI_NEW(MemoryKind::GfxApi) DX12DescriptorManagerImpl();
    for (u32 iHeap = 0; iHeap < kHeapTypeCount; ++iHeap)
    {
        self->PersistentAllocs[iHeap].mProvider.Initialize((D3D12_DESCRIPTOR_HEAP_TYPE)iHeap, false);
        self->TmpAllocs[iHeap].mProvider.Initialize((D3D12_DESCRIPTOR_HEAP_TYPE)iHeap, true);

        mHeapStepSize[iHeap] =
            gDX12GlobalState.Singletons.D3DDevice->GetDescriptorHandleIncrementSize((D3D12_DESCRIPTOR_HEAP_TYPE)iHeap);
    }
    return self;
}
void DX12DescriptorManager::Destroy()
{
    DX12DescriptorManagerImpl* self = DX12DescriptorManagerImpl::GetCombinePtr(this);
    for (u32 iHeap = 0; iHeap < kHeapTypeCount; ++iHeap)
    {
        self->PersistentAllocs[iHeap].Cleanup();
        self->PersistentAllocs[iHeap].mProvider.Finalize();
        self->TmpAllocs[iHeap].Cleanup();
        self->TmpAllocs[iHeap].mProvider.Finalize();
    }
    OMNI_DELETE(self, MemoryKind::GfxApi);
}
void DX12DescriptorManager::AllocRange(D3D12_DESCRIPTOR_HEAP_TYPE type,
    bool                                                          shaderVisible,
    u32                                                           count,
    ExternalAllocationHandle&                                     allocHandle,
    D3D12_CPU_DESCRIPTOR_HANDLE&                                  cpuDescHandle,
    D3D12_GPU_DESCRIPTOR_HANDLE&                                  gpuDescHandle)
{
    DX12DescriptorManagerImpl* self = DX12DescriptorManagerImpl::GetCombinePtr(this);
    ExternalAllocation         ea;
    u32                        iType = (u32)type;
    if (shaderVisible)
    {
        ea = self->TmpAllocs[iType].Alloc(count, 1);
    }
    else
    {
        ea = self->PersistentAllocs[iType].Alloc(count, 1);
    }
    auto* heap = (ID3D12DescriptorHeap*)ea.BlockId;
    cpuDescHandle.ptr = heap->GetCPUDescriptorHandleForHeapStart().ptr + count * mHeapStepSize[(u32)type];
    gpuDescHandle.ptr = heap->GetGPUDescriptorHandleForHeapStart().ptr + count * mHeapStepSize[(u32)type];
    allocHandle = ea.Handle;
}
void DX12DescriptorManager::FreeRange(
    D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible, ExternalAllocationHandle allocHandle)
{
    DX12DescriptorManagerImpl* self = DX12DescriptorManagerImpl::GetCombinePtr(this);
    u32                        iType = (u32)type;
    if (shaderVisible)
    {
        self->TmpAllocs[iType].Free(allocHandle);
    }
    else
    {
        self->PersistentAllocs[iType].Free(allocHandle);
    }
}
void DX12DescriptorMemProvider::Initialize(D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible)
{
    mType = type;
    mShaderVisible = shaderVisible;
}
void DX12DescriptorMemProvider::Finalize()
{
    CheckDebug(mUsedCount == 0, "allocator destroyed while there's still living allocations");
}
void DX12DescriptorMemProvider::Map(u64 size, u64 align, ExtenralAllocationBlockId& outBlockId)
{
    CheckDebug(align == 1);
    D3D12_DESCRIPTOR_HEAP_DESC desc;
    desc.Type = mType;
    desc.NumDescriptors = (u32)size;
    desc.Flags = mShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;

    ID3D12DescriptorHeap* descHeap;
    CheckDX12(gDX12GlobalState.Singletons.D3DDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descHeap)));
    outBlockId = (ExtenralAllocationBlockId)descHeap;
    mUsedCount += (u32)size;
}
void DX12DescriptorMemProvider::Unmap(ExtenralAllocationBlockId blockId)
{
    auto* descHeap = (ID3D12DescriptorHeap*)blockId;
#if OMNI_DEBUG
    mUsedCount -= descHeap->GetDesc().NumDescriptors;
#endif
    descHeap->Release();
}
u64 DX12DescriptorMemProvider::SuggestNewBlockSize(u64 reqSize)
{
    (void)reqSize;
    return 8 * 1024;
}
PMRAllocator DX12DescriptorMemProvider::GetCPUAllocator()
{
    return MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi);
}
} // namespace Omni

#endif // OMNI_WINDOWS
