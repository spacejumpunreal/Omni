#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"

namespace Omni
{
u32 DecodeQueueTypeMask(GfxApiQueueMask inMask, GfxApiQueueType outTypes[]);
}
