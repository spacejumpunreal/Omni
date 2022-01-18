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
    ExternalAllocationHandle  Handle;
    ExtenralAllocationBlockId BlockId;
};

} // namespace Omni
