#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Misc/AssertUtils.h"


#define CheckGfxApi(result) CheckAlways(SUCCEEDED(result))

namespace Omni
{
	void WaitGPUIdle();
}

#endif//OMNI_WINDOWS