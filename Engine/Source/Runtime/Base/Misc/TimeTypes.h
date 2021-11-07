#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include <chrono>

namespace Omni
{
	using TClock = std::chrono::steady_clock;
    using TimePoint = TClock::time_point;
    using Duration = TClock::duration;
}