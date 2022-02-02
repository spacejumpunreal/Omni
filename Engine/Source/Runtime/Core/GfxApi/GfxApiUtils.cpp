#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/GfxApi/GfxApiUtils.h"
#include "Runtime/Base/Math/SepcialFunctions.h"

namespace Omni
{
u32 DecodeQueueTypeMask(GfxApiQueueMask inMask, GfxApiQueueType outTypes[])
{
    u32              typeCount = 0;
    while (inMask != 0)
    {
        u32 bitIdx = Mathf::FindMostSignificant1Bit(inMask);
        outTypes[typeCount] = (GfxApiQueueType)bitIdx;
        ++typeCount;
        inMask = inMask & ~(GfxApiQueueMask(1) << bitIdx);
    }
    return typeCount;
}
}
