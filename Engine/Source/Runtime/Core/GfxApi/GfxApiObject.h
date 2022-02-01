#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Misc/EnumUtils.h"
#include "Runtime/Base/Misc/SharedObject.h"
#include "Runtime/Base/Memory/ObjectHandle.h"
#include "Runtime/Base/Math/Vector4.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/Platform/WindowUtils.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"
#include "Runtime/Core/GfxApi/GfxApiObjectHelper.h"
#include "Runtime/Core/GfxApi/GfxApiShader.h"

#include <variant>

namespace Omni
{
/**
 * forward decls
 */
// AsyncActions
class GfxApiBlitPass;
class GfxApiRenderPass;
class GfxApiComputePass;

/**
 * typedefs
 */
DECLARE_GFXAPI_REF_TYPE(GfxApiBufferRef, RawPtrHandle);
DECLARE_GFXAPI_REF_TYPE(GfxApiTextureRef, RawPtrHandle);
DECLARE_GFXAPI_REF_TYPE(GfxApiSwapChainRef, RawPtrHandle);
DECLARE_GFXAPI_REF_TYPE(GfxApiGpuEventRef, RawPtrHandle);
DECLARE_GFXAPI_REF_TYPE(GfxApiShaderRef, RawPtrHandle);

/**
 * enums
 */

/**
 * GfxApiObjectDesc definitions & GfxApiObject interface declarations
 */

// GfxApiObjectDesc
struct GfxApiObjectDesc
{
public:
    GfxApiObjectType Type;
    const char*      Name;

public:
    GfxApiObjectDesc(GfxApiObjectType type, const char* name = nullptr) : Type(type), Name(name)
    {
    }
};

/**
 * Resources
 */

// GfxApiBuffer
struct GfxApiBufferDesc : public GfxApiObjectDesc
{
public:
    u32               Size = 0;
    u32               Align = 0;
    GfxApiAccessFlags AccessFlags;

public:
    GfxApiBufferDesc() : GfxApiObjectDesc(GfxApiObjectType::Buffer)
    {
    }
};

// GfxApiTexture
struct GfxApiTextureDesc : public GfxApiObjectDesc
{
public:
    u32               Width;
    u32               Height;
    GfxApiAccessFlags AccessFlags;
    GfxApiFormat      Format;

public:
    GfxApiTextureDesc() : GfxApiObjectDesc(GfxApiObjectType::Texture)
    {
    }
};

// GfxApiSwapChain
struct GfxApiSwapChainDesc : public GfxApiObjectDesc
{
public:
    u32          BufferCount;
    u32          Width;
    u32          Height;
    GfxApiFormat Format;
    WindowHandle WindowHandle;

public:
    GfxApiSwapChainDesc()
        : GfxApiObjectDesc(GfxApiObjectType::Swapchain)
        , BufferCount(3)
        , Width(0)
        , Height(0)
        , Format(GfxApiFormat::R8G8B8A8_UNORM)
    {
    }
};

/**
 * GPU program, PSO related
 */

// GfxApiShader
struct GfxApiShaderDesc : public GfxApiObjectDesc
{
public:
    std::string_view           Source;
    const u8*                  BinaryPtr;
    size_t                     BinarySize;
    const char*                EntryName;
    GfxApiShaderStage          Stage;
    GfxApiShaderCompileOptions Options;

public:
    GfxApiShaderDesc()
        : GfxApiObjectDesc(GfxApiObjectType::Shader)
        , Source(nullptr, 0)
        , BinaryPtr(nullptr)
        , BinarySize(0)
        , EntryName(nullptr)
        , Stage(GfxApiShaderStage::Vertex)
    {
    }
};



} // namespace Omni
