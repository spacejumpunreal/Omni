#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Misc/AssertUtils.h"

namespace Omni
{
	FORCEINLINE void CheckGfxApi(HRESULT result)
	{
		CheckAlways(SUCCEEDED(result));
	}
}

#endif//OMNI_WINDOWS