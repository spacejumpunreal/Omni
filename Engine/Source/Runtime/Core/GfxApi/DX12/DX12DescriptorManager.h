#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Memory/ExternalAllocation.h"

/*
 * forward declarations
 */
enum D3D12_DESCRIPTOR_HEAP_TYPE;

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
        D3D12_GPU_DESCRIPTOR_HANDLE&           gpuDescHandle);
    void FreeRange(D3D12_DESCRIPTOR_HEAP_TYPE type, bool shaderVisible, ExternalAllocationHandle allocHandle);
    
    static u32 GetHeapIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type)
    {
        return mHeapStepSize[(u32)type];
    }

private:
    static u32 mHeapStepSize[4];
};
} // namespace Omni

#endif // OMNI_WINDOWS
