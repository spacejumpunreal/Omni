#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include <cstdarg>
#include <cstdio>

namespace Omni
{
	void BASE_API BreakAndCrash(volatile int*);
#if OMNI_WINDOWS
	void BASE_API CheckWinAPI(int x);
#endif
	void BASE_API OmniDebugBreak();

	template<typename T>
	inline void OmniCheck(const T& v, const char* fmt, ...)
	{
		if (!v)
		{
			va_list args;
			va_start(args, fmt);
			vprintf(fmt, args);
			va_end(args);
			fflush(0);
			OmniDebugBreak();
			BreakAndCrash(nullptr);
		}

	}
	template<typename T>
	inline void OmniCheck(const T& v, ...)
	{
		if (!v)
		{
			OmniDebugBreak();
			BreakAndCrash(nullptr);
		}
	}
	inline void NotImplemented()
	{
		BreakAndCrash(nullptr);
	}

}

#define NotImplemented(...) OmniCheck(false, ##__VA_ARGS__)
#define CheckAlways(cond, ...) OmniCheck((cond), ##__VA_ARGS__)
#define CheckSucceeded(cond, ...) OmniCheck(SUCCEEDED(cond), ##__VA_ARGS__)

#if OMNI_DEBUG
#define CheckDebug(cond, ...) OmniCheck((cond), ##__VA_ARGS__)
#else
#define CheckDebug(cond, ...) ((void)(cond))
#endif

