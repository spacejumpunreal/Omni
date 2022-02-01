#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"
#include "Runtime/Base/Memory/HandleObjectPool.h"
#include "Runtime/Core/GfxApi/GfxApiObjectHelper.h"

namespace Omni
{
/*
 * references:
 * https://developer.apple.com/documentation/metal/mtlrenderpipelinedescriptor?language=objc
 * https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_graphics_pipeline_state_desc
 */

struct GfxApiRTBlendConfig
{
    u8                WriteMask; // rgba = bit3210
    GfxApiBlendOps    ColorOp;
    GfxApiBlendFactor SrcColorFactor;
    GfxApiBlendFactor DstColorFactor;
    GfxApiBlendOps    AlphaOp;
    GfxApiBlendFactor SrcAlphaFactor;
    GfxApiBlendFactor DstAlphaFactor;
    bool              EnableBlend;
};

struct GfxApiBlendStateDesc
{
    GfxApiRTBlendConfig Configs[kMaxMRTCount];
};

struct GfxApiRasterizerStateDesc
{ // https://github.com/gpuweb/gpuweb/issues/137
    GfxApiFillMode      FillMode;
    GfxApiCullMode      CullMode;
    /*
     * ref
     * https://docs.microsoft.com/en-us/windows/win32/direct3d11/d3d10-graphics-programming-guide-output-merger-stage-depth-bias
     */
    i32                 DepthBias;
    float               DepthBiasClamp;
    float               SlopeScaledDepthBias;
    GfxApiDepthClipMode DepthClipMode;
    bool                EnableRasterization;
};

struct GfxApiStencilDesc
{
    GfxApiTestFunc   TestFunc;
    GfxApiStencilOps StencilFailOp;
    GfxApiStencilOps DepthFailOp; // stencil already passed, but depth failed
    GfxApiStencilOps PassOp;      // both passed
};

struct GfxApiDepthStencilStateDesc
{ // https://github.com/gpuweb/gpuweb/issues/118
    GfxApiTestFunc    DepthFunc;
    GfxApiStencilDesc StencilFront;
    GfxApiStencilDesc StencilBack;
    u8                StencilReadMask;
    u8                StencilWriteMask;
    bool              EnableDepth;
    bool              EnableStencil;
};

DECLARE_GFXAPI_REF_TYPE(GfxApiBlendStateRef, RawPtrHandle);
DECLARE_GFXAPI_REF_TYPE(GfxApiRasterizerStateRef, RawPtrHandle);
DECLARE_GFXAPI_REF_TYPE(GfxApiDepthStencilStateRef, RawPtrHandle);

} // namespace Omni

#endif // OMNI_WINDOWS
