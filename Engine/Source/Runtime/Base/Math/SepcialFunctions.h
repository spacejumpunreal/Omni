#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include <cmath>


namespace Omni
{
	struct Mathf
	{
		FORCEINLINE static float Sqrtf(float f)
		{
			return std::sqrtf(f);
		}
		FORCEINLINE static bool IsNan(float f)
		{
			return std::isnan(f);
		}
		FORCEINLINE static float Abs(float f)
		{
			return std::abs(f);
		}
		FORCEINLINE static i32 Abs(i32 i)
		{
			return std::abs(i);
		}
		FORCEINLINE static i64 Abs(i64 ll)
		{
			return std::abs(ll);
		}
	};
}