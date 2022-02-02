#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Container/LinkedList.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Base/Memory/PageSubAllocator.h"


namespace Omni
{
/**
 * forward decls
 */

/**
 * typedefs
 */
using TClearValue = std::variant<u32, u8, Vector4, float>;

/**
 * enums
 */

/**
 * definitions
 */
struct GfxApiCopyBuffer
{
    GfxApiBufferRef Src;
    u64             SrcOffset;
    GfxApiBufferRef Dst;
    u64             DstOffset;
    u64             Bytes;
};


class GfxApiBlitPass
{
public:
    CORE_API GfxApiBlitPass(PageSubAllocator* alloc, u32 capacity = 0);
public:
    GfxApiCopyBuffer* CopyBufferCmds;
    u32               CopyBufferCmdCount;


};

} // namespace Omni
