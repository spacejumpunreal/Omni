#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12Resource.h"
#include "Runtime/Core/Platform/OSUtils_Windows.h"
#include "Runtime/Core/GfxApi/DX12/d3dx12.h"

namespace Omni
{

D3D12_RESOURCE_STATES DX12Resource::CalcInitialStateFromAccessFlag(GfxApiAccessFlags accessFlag)
{
    switch (accessFlag)
    {
    default:
    case GfxApiAccessFlags::GPUPrivate:
        return D3D12_RESOURCE_STATE_COMMON;
    case GfxApiAccessFlags::Readback:
        return D3D12_RESOURCE_STATE_COPY_DEST;
    case GfxApiAccessFlags::Upload:
        return D3D12_RESOURCE_STATE_GENERIC_READ;
    }
}
DX12Resource::DX12Resource(D3D12_RESOURCE_STATES resState) 
    : mDX12Resource(nullptr)
    , mResourceState(resState)
{
}
DX12Resource::~DX12Resource()
{
    if (mDX12Resource)
        SafeRelease(mDX12Resource);
}
bool DX12Resource::EmitBarrier(D3D12_RESOURCE_STATES newState, D3D12_RESOURCE_BARRIER* barrier)
{
    if (mResourceState == newState)
        return false;
    *barrier = CD3DX12_RESOURCE_BARRIER::Transition(mDX12Resource,
                                                    mResourceState,
                                                    newState,
                                                    0,
                                                    D3D12_RESOURCE_BARRIER_FLAGS::D3D12_RESOURCE_BARRIER_FLAG_NONE);
    mResourceState = newState;
    return true;
}

} // namespace Omni

#endif
