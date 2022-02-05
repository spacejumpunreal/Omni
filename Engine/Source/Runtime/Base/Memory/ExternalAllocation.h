#pragma once
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Prelude/Omni.h"

namespace Omni
{

using ExternalAllocationHandle = void*;
using ExtenralAllocationBlockId = void*;
struct ExternalAllocation
{
    u64                       Start;
    u64                       Size;
    ExternalAllocationHandle  Handle; //the id for this allocation, used for free
    ExtenralAllocationBlockId BlockId;//the id of external memory resource, ID3D12Heap*/ID3D12DescriptorHeap* for example
};

} // namespace Omni
