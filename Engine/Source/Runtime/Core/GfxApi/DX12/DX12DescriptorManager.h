#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Base/Memory/ExternalAllocation.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/DX12/DX12ForwardDecl.h"

/*
 * forward declarations
 */
enum D3D12_DESCRIPTOR_HEAP_TYPE;
struct ID3D12DescriptorHeap;

namespace Omni
{
/*
 * forward declarations
 */

class DX12DescriptorManager
{
public:
    static DX12DescriptorManager* Create();
    void                          Destroy();

    void AllocRange(D3D12_DESCRIPTOR_HEAP_TYPE type,
        bool                                   shaderVisible,
        u32                                    count,
        ExternalAllocationHandle&              allocHandle,
        D3D12_CPU_DESCRIPTOR_HANDLE&           cpuDescHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE&           gpuDescHandle,
        ID3D12DescriptorHeap*&                 heap);
    void FreeRange(D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible, ExternalAllocationHandle allocHandle);

    static u32 GetHeapIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type)
    {
        return mHeapStepSize[(u32)type];
    }

private:
    static u32 mHeapStepSize[4];
};

class DX12DescriptorTmpAllocator
{ // alloc large page each time and batch free them
public:
    DEFINE_GFX_API_TEMP_NEW_DELETE();
    DX12DescriptorTmpAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, u32 batchSize);
    ~DX12DescriptorTmpAllocator();
    void EnsureSpace(u32 count);
    void Alloc(u32 count, D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle);

private:
    ID3D12DescriptorHeap*               mCurHeap;
    u64                                 mCurCPUPtr;
    u64                                 mCurGPUPtr;
    u32                                 mStep;
    u32                                 mUsed;
    u32                                 mBatchSize;
    D3D12_DESCRIPTOR_HEAP_TYPE          mType;
    PMRVector<ExternalAllocationHandle> mPages;
};

} // namespace Omni

#endif // OMNI_WINDOWS
