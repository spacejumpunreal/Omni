#pragma once
#include "Runtime/Base/Misc/EnumUtils.h"
#include "Runtime/Prelude/Omni.h"

namespace Omni
{
/**
 * Constants
 */
constexpr u32 kMaxMRTCount = 8;

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
    D24_UNORM_S8_UINT,
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
constexpr GfxApiQueueMask kAllQueueMask = (1 << (u32)GfxApiQueueType::GraphicsQueue) |
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

/*
 * blend related
 */
enum class GfxApiBlendOps : u8
{ // FinalValue = BlendOp(SrcBlendFactor * SrcAlpha, DstBlendFactor * DstAlpha)
    Add,
    Subtract,        // Arg0 - Arg1
    ReverseSubtract, // Arg1 - Arg0
    Min,
    Max,
};

enum class GfxApiBlendFactor : u8
{
    // this is the same for dx12/vulkan/metal, although for dx12 Zero's int value is 1
    // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_blend
    // https://developer.apple.com/documentation/metal/mtlblendfactor?language=objc
    // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkBlendFactor.html
    Zero,
    One,
    SourceColor,
    OneMinusSourceColor,
    SourceAlpha,
    OneMinusSourceAlpha,
    DstinationColor,
    OneMinusDestinationColor,
    DestinationAlpha,
    OneMinusDestinationAlpha,
    SourceAlphaSaturated,
    BlendColor,
    OneMinusBlendColor,
    BlendAlpha,
    OneMinusBlendAlpha,
    Source1Color,
    OneMinusSource1Color,
    Source1Alpha,
    OneMinusSource1Alpha,
};

/*
 * rasterizer related
 */
enum class GfxApiFillMode : u8
{
    Wireframe,
    Solid,
};

enum class GfxApiCullMode : u8
{
    CullNone,
    CullFrontFace,
    CullBackFace,
};

enum class GfxApiDepthClipMode : u8
{
    Clip,
    Clamp,
};

/*
 * depth stencil related
 */
enum class GfxApiTestFunc : u8
{
    Never,
    Less,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always,
};

enum class GfxApiStencilOps
{
    Keep,
    Zero,
    Replace,
    IncClamp,
    DecClamp,
    Invert,
    IncWrap,
    DecWrap,
};

} // namespace Omni
