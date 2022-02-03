#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiDefs.h"
#include "Runtime/Core/GfxApi/GfxApiBinding.h"
#include "Runtime/Core/GfxApi/DX12/DX12ForwardDecl.h"
#include "Runtime/Core/GfxApi/GfxApiRenderPass.h"

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
    GfxApiPSOParams            Params;
    GfxApiFormat               RTFormats[kMaxMRTCount];
    GfxApiFormat               DSFormat;
    u8                         RTCount;
};

class DX12PSOManager
{
public:
    static DX12PSOManager* Create();
    void                   Destroy();
    ID3D12PipelineState*   GetOrCreatePSO(const DX12PSOKey& key);
    void                   PurgePSO(const GfxApiPurgePSOOptions& options);
};

} // namespace Omni
#endif // OMNI_WINDOWS
