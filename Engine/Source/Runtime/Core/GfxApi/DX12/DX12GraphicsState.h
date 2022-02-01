#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/GfxApi/GfxApiGraphicState.h"
#if OMNI_WINDOWS

#include <d3d12.h>

namespace Omni
{
/*
 * constants
 */

/*
 * declarations
 */
class DX12BlendState : public D3D12_BLEND_DESC
{
public:
    DX12BlendState(const GfxApiBlendStateDesc& desc);
};

class DX12RasterizerState : public D3D12_RASTERIZER_DESC
{
public:
    DX12RasterizerState(const GfxApiRasterizerStateDesc& desc);
};

class DX12DepthStencilState : public D3D12_DEPTH_STENCIL_DESC
{
public:
    DX12DepthStencilState(const GfxApiDepthStencilStateDesc& desc);
};

}

#endif

