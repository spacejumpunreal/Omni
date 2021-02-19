#pragma once
#include "Runtime/Omni.h"

#if OMNI_CLANG
#include <experimental/vector>
#else
#include <vector>
#endif

namespace Omni
{
#if OMNI_CLANG
	template<typename T>
	using PMRVector = std::experimental::pmr::vector<T>;
#else
	template<typename T>
	using PMRVector = std::pmr::vector<T>;
#endif
}
