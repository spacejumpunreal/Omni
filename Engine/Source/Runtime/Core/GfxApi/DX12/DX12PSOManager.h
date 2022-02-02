#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiDefs.h"
#include "Runtime/Core/GfxApi/GfxApiGraphicState.h"
#include "Runtime/Core/GfxApi/GfxApiBinding.h"
#include "Runtime/Core/GfxApi/DX12/DX12ForwardDecl.h"

namespace Omni
{
/*
 * forward decls
 */
class DX12BlendState;
class DX12RasterizerState;
class DX12DepthStencilState;

/*
 * declaration
 */
struct DX12PSOKey
{
    GfxApiPSOSignatureRef      Signature;
    GfxApiBlendStateRef        BlendState;
    GfxApiRasterizerStateRef   RSState;
    GfxApiDepthStencilStateRef DSState;
    GfxApiShaderRef            Shaders[(u32)GfxApiShaderStage::GraphicsCount];
    GfxApiFormat               RTFormats[kMaxMRTCount];
    GfxApiFormat               DSFormat;
    u8                         RTCount;
};

class DX12PSOManager
{
public:
    static DX12PSOManager* Create();
    void                   Destroy();
    ID3D12PipelineState*   Get(const DX12PSOKey& key);
    void                   Perge();
};

} // namespace Omni
#endif // OMNI_WINDOWS
