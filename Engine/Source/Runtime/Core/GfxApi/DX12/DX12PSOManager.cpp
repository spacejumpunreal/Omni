#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/GfxApi/DX12/DX12PSOManager.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"

namespace Omni
{
/**
 * forward decls
 */

/**
 * declarations
 */
struct DX12PSOManagerPrivateData
{
};
using DX12PSOManagerImpl = PImplCombine<DX12PSOManager, DX12PSOManagerPrivateData>;

/**
 * definitions
 */
DX12PSOManager* DX12PSOManager::Create()
{
    return OMNI_NEW(MemoryKind::GfxApi) DX12PSOManagerImpl();
}

void DX12PSOManager::Destroy()
{
    auto* self = DX12PSOManagerImpl::GetCombinePtr(this);
    OMNI_DELETE(self, MemoryKind::GfxApi);
}

} // namespace Omni
