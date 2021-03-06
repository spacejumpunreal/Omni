#pragma once
#include "Runtime/Base/BaseAPI.h"

namespace Omni
{
	template<typename T>
	static constexpr T CompileTimeLog2(T x)
	{
		return x == 1 ? 0 : CompileTimeLog2(x >> 1) + 1;
	}
}

