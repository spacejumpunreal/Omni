#pragma once
#include "Runtime/Base/Misc/EnumUtils.h"
#include "Runtime/Prelude/Omni.h"

namespace Omni
{
/**
 * Constants
 */
constexpr u32 MaxMRTCount = 8;

/**
 * Enums
 */
enum class GfxApiObjectType : u32
{
    Buffer,
    Texture,
    Sampler,
    Pso,
    Shader,
    Fence,
    Swapchain,
};

enum class GfxApiFormat : u32
{
    R8G8B8A8_UNORM,
    R16G16B16A16_FLOAT,
    R11G11B10_FLOAT,
};

enum class GfxApiAccessFlags : u32
{
    CPURead = 1 << 0,
    CPUWrite = 1 << 1,
    GPURead = 1 << 2,
    GPUWrite = 1 << 3,
};
DEFINE_BITMASK_ENUM_OPS(GfxApiAccessFlags);

enum class GfxApiQueueType : u8
{
    GraphicsQueue,
    ComputeQueue,
    CopyQueue,
    Count,
};

using GfxApiQueueMask = u32;
constexpr GfxApiQueueMask AllQueueMask = 1 << (u32)GfxApiQueueType::GraphicsQueue;
static_assert(sizeof(GfxApiQueueMask) * 8 > (size_t)GfxApiQueueType::Count);

enum class GfxApiLoadStoreActions : u32
{
    Load = 1 << 0,
    Clear = 1 << 1,
    NoAccess = 1 << 2,
    Store = 1 << 3,
    Discard = 1 << 4,
};
DEFINE_BITMASK_ENUM_OPS(GfxApiLoadStoreActions);
} // namespace Omni
