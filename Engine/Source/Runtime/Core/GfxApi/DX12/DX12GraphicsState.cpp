#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12GraphicsState.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Base/Misc/AssertUtils.h"

namespace Omni
{
/*
 * constants
 */

/*
 * definitions
 */

static u8 ToDX12WriteMask(u8 mask)
{
    u8 acc = 0;
    if (mask & 0x1)
        acc |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
    if (mask & 0x2)
        acc |= D3D12_COLOR_WRITE_ENABLE_BLUE;
    if (mask & 0x4)
        acc |= D3D12_COLOR_WRITE_ENABLE_GREEN;
    if (mask & 0x8)
        acc |= D3D12_COLOR_WRITE_ENABLE_RED;
    return acc;
}

DX12BlendState::DX12BlendState(const GfxApiBlendStateDesc& desc)
{
    ZeroFill(*this);
    for (u32 iRT = 0; iRT < kMaxMRTCount; ++iRT)
    {
        D3D12_RENDER_TARGET_BLEND_DESC& rtDesc = RenderTarget[iRT];
        const GfxApiRTBlendConfig&            rtConfig = desc.Configs[iRT];
        rtDesc.BlendEnable = desc.Configs[iRT].EnableBlend ? TRUE : FALSE;
        rtDesc.LogicOpEnable = FALSE;
        rtDesc.RenderTargetWriteMask = ToDX12WriteMask(rtConfig.WriteMask);
        if (desc.Configs[iRT].EnableBlend)
            NotImplemented();
    }
}

DX12RasterizerState::DX12RasterizerState(const GfxApiRasterizerStateDesc& desc)
{
    ZeroFill(*this);
    FillMode = desc.FillMode == GfxApiFillMode::Solid ? D3D12_FILL_MODE_SOLID : D3D12_FILL_MODE_WIREFRAME;
    CullMode = (D3D12_CULL_MODE)(D3D12_CULL_MODE_NONE + (u8)desc.CullMode);
    DepthBias = desc.DepthBias;
    DepthBiasClamp = desc.DepthBiasClamp;
    SlopeScaledDepthBias = desc.SlopeScaledDepthBias;
    DepthClipEnable = desc.DepthClipMode == GfxApiDepthClipMode::Clip ? TRUE : FALSE;
    MultisampleEnable = FALSE;
    AntialiasedLineEnable = FALSE;
    ForcedSampleCount = 0;
}

DX12DepthStencilState::DX12DepthStencilState(const GfxApiDepthStencilStateDesc& desc)
{
    ZeroFill(*this);
    DepthEnable = desc.EnableDepth ? TRUE : FALSE;
    DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    DepthFunc = (D3D12_COMPARISON_FUNC)(u8(desc.DepthFunc) + 1);
    StencilEnable = desc.EnableStencil;
    if (StencilEnable)
        NotImplemented();
}

} // namespace Omni

#endif
