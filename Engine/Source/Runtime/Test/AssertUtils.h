#pragma once
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <vector>

namespace Omni
{
	void BreakAndCrash(volatile int*);
#if OMNI_WINDOWS
	void CheckWinAPI(int x);
#endif
	void OmniDebugBreak();

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

}

#define CheckAlways(cond, ...) OmniCheck((cond), ##__VA_ARGS__)
#define CheckSucceeded(cond, ...) OmniCheck(SUCCEEDED(cond), ##__VA_ARGS__)
#ifdef NDEBUG
#define CheckDebug(cond, ...) ((void)(cond))
#else
#define CheckDebug(cond, ...) OmniCheck((cond), ##__VA_ARGS__)
#endif
