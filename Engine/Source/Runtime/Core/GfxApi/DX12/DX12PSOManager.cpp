#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/GfxApi/DX12/DX12PSOManager.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/Misc/HashUtils.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12DeleteManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Shader.h"
#include "Runtime/Core/GfxApi/DX12/DX12GraphicsState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"

namespace Omni
{
/**
 * forward decls
 */

/**
 * declarations
 */
struct DX12PSOKeyHasher
{
    size_t operator()(const DX12PSOKey& key) const
    {
        return ComputeHash((u8*)&key, sizeof(DX12PSOKey));
    }
};

struct DX12PSOKeyEqualTo
{
    bool operator()(const DX12PSOKey& l, const DX12PSOKey& r) const
    {
        return memcmp(&l, &r, sizeof(DX12PSOKey));
    }
};

struct DX12PSOPurgeReq
{
public:
    DEFINE_GFX_API_TEMP_NEW_DELETE();
    DX12PSOPurgeReq(u32 count);
    ~DX12PSOPurgeReq();

public:
    ID3D12PipelineState** PSOs;
    u32                   PSOCount;
};

struct DX12PSOManagerPrivateData
{
public:
    DX12PSOManagerPrivateData();

public:
    PMRUnorderedMap<DX12PSOKey, ID3D12PipelineState*, DX12PSOKeyHasher, DX12PSOKeyEqualTo> Cache;
};
using DX12PSOManagerImpl = PImplCombine<DX12PSOManager, DX12PSOManagerPrivateData>;

/**
 * definitions
 */

// DX12PSOPurgeReq
DX12PSOPurgeReq::DX12PSOPurgeReq(u32 count) : PSOCount(count)
{
    auto talloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApiTmp);
    PSOs = talloc.allocate_object<ID3D12PipelineState*>(count);
}

DX12PSOPurgeReq::~DX12PSOPurgeReq()
{
    for (u32 iPSO = 0; iPSO < PSOCount; ++iPSO)
    {
        PSOs[iPSO]->Release();
    }
    auto talloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApiTmp);
    talloc.deallocate_object<ID3D12PipelineState*>(PSOs, PSOCount);
}

// DX12PSOManagerPrivateData
DX12PSOManagerPrivateData::DX12PSOManagerPrivateData() : Cache(MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi))
{
}

// DX12PSOManager
DX12PSOManager* DX12PSOManager::Create()
{
    return OMNI_NEW(MemoryKind::GfxApi) DX12PSOManagerImpl();
}

void DX12PSOManager::Destroy()
{
    auto*                 self = DX12PSOManagerImpl::GetCombinePtr(this);
    GfxApiPurgePSOOptions options;
    PurgePSO(options);
    OMNI_DELETE(self, MemoryKind::GfxApi);
}

static ID3D12PipelineState* CreatePSOFromKey(const DX12PSOKey& key)
{
    auto& shaderPool = gDX12GlobalState.DX12ShaderPool;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC gDesc;
    memset(&gDesc, 0, sizeof(gDesc));

    gDesc.pRootSignature = gDX12GlobalState.DX12PSOSignaturePool.ToPtr(key.Params.Signature)->GetRootSignature();

    ID3DBlob* shaderCode;
    shaderCode = shaderPool.ToPtr(key.Params.Shaders[(u32)GfxApiShaderStage::Vertex])->GetCompiledBinary();
    gDesc.VS.pShaderBytecode = shaderCode->GetBufferPointer();
    gDesc.VS.BytecodeLength = shaderCode->GetBufferSize();

    shaderCode = shaderPool.ToPtr(key.Params.Shaders[(u32)GfxApiShaderStage::Fragment])->GetCompiledBinary();
    gDesc.PS.pShaderBytecode = shaderCode->GetBufferPointer();
    gDesc.PS.BytecodeLength = shaderCode->GetBufferSize();

    gDesc.BlendState =
        *static_cast<D3D12_BLEND_DESC*>(gDX12GlobalState.DX12BlendStatePool.ToPtr(key.Params.BlendState));
    gDesc.RasterizerState = *static_cast<D3D12_RASTERIZER_DESC*>(
        gDX12GlobalState.DX12RasterizerStatePool.ToPtr(key.Params.RasterizerState));
    gDesc.DepthStencilState = *static_cast<D3D12_DEPTH_STENCIL_DESC*>(
        gDX12GlobalState.DX12DepthStencilStatePool.ToPtr(key.Params.DepthStencilState));
    gDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    gDesc.SampleDesc.Count = 1;
    gDesc.SampleDesc.Quality = 0;
    gDesc.NumRenderTargets = key.RTCount;
    for (u32 iRT = 0; iRT < key.RTCount; ++iRT)
    {
        gDesc.RTVFormats[iRT] = ToDXGIFormat(key.RTFormats[iRT]);
    }
    gDesc.DSVFormat = ToDXGIFormat(key.DSFormat);
    ID3D12PipelineState* ret;
    CheckDX12(gDX12GlobalState.Singletons.D3DDevice->CreateGraphicsPipelineState(&gDesc, IID_PPV_ARGS(&ret)));
    return ret;
}

ID3D12PipelineState* DX12PSOManager::GetOrCreatePSO(const DX12PSOKey& key)
{
    auto*                self = DX12PSOManagerImpl::GetCombinePtr(this);
    ID3D12PipelineState* pso;
    auto                 it = self->Cache.find(key);
    if (it == self->Cache.end())
    {
        pso = CreatePSOFromKey(key);
        self->Cache.insert(std::make_pair(key, pso));
        return pso;
    }
    else
    {
        return it->second;
    }
}

void DX12PSOManager::PurgePSO(const GfxApiPurgePSOOptions& options)
{
    (void)options;
    auto*            self = DX12PSOManagerImpl::GetCombinePtr(this);
    DX12PSOPurgeReq* req = new DX12PSOPurgeReq((u32)self->Cache.size());
    u32              cnt = 0;
    for (auto& pair : self->Cache)
    {
        req->PSOs[cnt++] = pair.second;
    }
    self->Cache.clear();
    gDX12GlobalState.DeleteManager->AddForDelete(req, kAllQueueMask);
}

} // namespace Omni
