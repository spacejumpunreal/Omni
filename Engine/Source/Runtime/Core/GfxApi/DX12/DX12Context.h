#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include <d3d12.h>
#include <dxgi1_6.h>


namespace Omni
{
	struct DX12Context
	{
	public:
		DX12Context();
	public:
		IDXGIFactory4* Factory;
	};
}

#endif//OMNI_WINDOWS