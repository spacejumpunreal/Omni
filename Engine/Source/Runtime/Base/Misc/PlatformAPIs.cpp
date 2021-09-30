#include "Runtime/Base/BasePCH.h"
#include "Runtime/Base/Misc/PlatformAPIs.h"
#include "Runtime/Base/Misc/ArrayUtils.h"

#if OMNI_WINDOWS
#include <Windows.h>
#endif

namespace Omni
{
	void PauseThread()
	{
#if OMNI_WINDOWS
		YieldProcessor();
#else
        __asm__ __volatile__("yield");
#endif
	}

}
