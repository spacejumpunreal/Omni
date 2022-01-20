#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12ForwardDecl.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"


namespace Omni
{
	class DX12Resource
	{
	public:
        static D3D12_RESOURCE_STATES CalcInitialStateFromAccessFlag(GfxApiAccessFlags accessFlag);
        DX12Resource(D3D12_RESOURCE_STATES resState);
        ~DX12Resource();
        ID3D12Resource* GetResource() { return mDX12Resource; }
        bool EmitBarrier(D3D12_RESOURCE_STATES newState, D3D12_RESOURCE_BARRIER* barrier);

	protected:
		ID3D12Resource*		        mDX12Resource;
        D3D12_RESOURCE_STATES       mResourceState;
	};
}

#endif//OMNI_WINDOWS
