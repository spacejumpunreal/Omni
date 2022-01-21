#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS

struct ID3D12GraphicsCommandList4;


namespace Omni
{
enum class GfxApiQueueType : u8;

ID3D12GraphicsCommandList4* SetupCommandList(GfxApiQueueType queueType);

} // namespace Omni

#endif // OMNI_WINDOWS
