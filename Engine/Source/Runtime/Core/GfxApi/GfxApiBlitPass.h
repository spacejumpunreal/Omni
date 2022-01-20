#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Container/LinkedList.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"


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
    DEFINE_GFX_API_TEMP_NEW_DELETE()
    CORE_API GfxApiBlitPass(u32 nCopyBufferCmds = 0);
public:
    PMRVector<GfxApiCopyBuffer> CopyBufferCmds;

};

} // namespace Omni
