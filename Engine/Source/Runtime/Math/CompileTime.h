#pragma once
#include "Runtime/Omni.h"

namespace Omni
{
	template<typename T>
	static constexpr T CompileTimeLog2(T x)
	{
		return x == 1 ? CompileTimeLog2(x >> 1) + 1 : 0;
	}
}