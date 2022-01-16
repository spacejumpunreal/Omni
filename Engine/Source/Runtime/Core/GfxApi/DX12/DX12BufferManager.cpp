#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12BufferManager.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/d3dx12.h"

namespace Omni
{
struct DX12BufferMangerPrivateData
{
};

using DX12BufferManagerImpl = PImplCombine<DX12BufferManager, DX12BufferMangerPrivateData>;

DX12BufferManager* DX12BufferManager::Create()
{
    return OMNI_NEW(MemoryKind::GfxApi) DX12BufferManagerImpl();
}

void DX12BufferManager::Destroy()
{
    DX12BufferManagerImpl* self = DX12BufferManagerImpl::GetCombinePtr(this);
    OMNI_DELETE(self, MemoryKind::GfxApi);
}

void DX12BufferManager::AllocBuffer(const GfxApiBufferDesc& desc, ID3D12Resource*& dx12Buffer,
                                    ExternalAllocationHandle& allocHandle)
{
    (void)desc;
    (void)dx12Buffer;
    (void)allocHandle;
    //DX12BufferManagerImpl* self = DX12BufferManagerImpl::GetCombinePtr(this);
}

} // namespace Omni

#endif // OMNI_WINDOWS
