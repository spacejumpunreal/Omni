#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12GraphicsState.h"
#include "Runtime/Base/Misc/AssertUtils.h"

namespace Omni
{
/*
 * constants
 */

/*
 * definitions
 */

DX12BlendState::DX12BlendState(const GfxApiBlendStateDesc& desc)
{
    memset(this, 0, sizeof(*this));
    for (u32 iRT = 0; iRT < kMaxMRTCount; ++iRT)
    {
        D3D12_RENDER_TARGET_BLEND_DESC& rtDesc = RenderTarget[iRT];
        rtDesc.BlendEnable = desc.Configs[iRT].EnableBlend ? TRUE : FALSE;
        if (desc.Configs[iRT].EnableBlend)
            NotImplemented();
    }
}

DX12RasterizerState::DX12RasterizerState(const GfxApiRasterizerStateDesc& desc)
{
    memset(this, 0, sizeof(*this));
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
    memset(this, 0, sizeof(*this));
    DepthEnable = desc.EnableDepth ? TRUE : FALSE;
    DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    DepthFunc = (D3D12_COMPARISON_FUNC)(u8(desc.DepthFunc) + 1);
    StencilEnable = desc.EnableStencil;
    if (StencilEnable)
        NotImplemented();
}

} // namespace Omni

#endif
