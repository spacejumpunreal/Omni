#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Misc/AssertUtils.h"


#define CheckDX12(result) CheckAlways(SUCCEEDED(result))

namespace Omni
{
	void WaitGPUIdle();
}

#endif//OMNI_WINDOWS
