#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Misc/EnumUtils.h"

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
        CPUAccess = 1 << 0,
        GPURead = 1 << 1,
        GPUWrite = 1 << 2,
    };

    enum class GfxApiQueueType : u8
    {
        GraphicsQueue,
        ComputeQueue,
        CopyQueue,
        Count,
    };
    enum class GfxApiLoadStoreActions : u32
    {
        Load = 1 << 0,
        Clear = 1 << 1,
        DontCare = 1 << 2,
        Store = 1 << 3,
        Discard = 1 << 4,
    };
    DEFINE_ENUM_CLASS_OPS(GfxApiLoadStoreActions);
}
