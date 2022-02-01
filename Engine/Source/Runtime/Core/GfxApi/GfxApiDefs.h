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
enum class GfxApiObjectType : u8
{
    Buffer,
    Texture,
    Sampler,
    Shader,
    Fence,
    Swapchain,
};

enum class GfxApiFormat : u8
{
    R8G8B8A8_UNORM,
    R16G16B16A16_FLOAT,
    R11G11B10_FLOAT,
};

enum class GfxApiAccessFlags : u8
{
    GPUPrivate,
    Upload,
    Readback,
    Count,
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
constexpr GfxApiQueueMask AllQueueMask = (1 << (u32)GfxApiQueueType::GraphicsQueue) |
                                         (1 << (u32)GfxApiQueueType::ComputeQueue) |
                                         (1 << (u32)GfxApiQueueType::CopyQueue);

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

enum class GfxApiShaderStage
{
    // Graphics
    Vertex = 0,
    Fragment = 1,
    GraphicsCount = 2,
    // Compute
    Compute = 2,
    ComputeCount = 1,
    TotalCount = 3,
};
DEFINE_BITMASK_ENUM_OPS(GfxApiShaderStage);

enum class GfxApiBindingGroupSlot : u8
{
    Pass = 0,
    Material,
    Instance,
    Count
};

#define DECLARE_GFXAPI_REF_TYPE(RefTypeName, BaseType)                                                                 \
    struct RefTypeName : public BaseType                                                                               \
    {                                                                                                                  \
        using UnderlyingHandle = BaseType;                                                                             \
    }

} // namespace Omni
