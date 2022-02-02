#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"

/*
 * forward decls
 */
enum D3D12_COMMAND_LIST_TYPE;
enum DXGI_FORMAT;

/*
 * macros
 */
#define CheckDX12(result) CheckAlways(SUCCEEDED(result))

namespace Omni
{
D3D12_COMMAND_LIST_TYPE QueueTypeToCmdType(GfxApiQueueType queueType);
DXGI_FORMAT             ToDXGIFormat(GfxApiFormat format);
} // namespace Omni

#endif // OMNI_WINDOWS
