#pragma once

#include <vector>
#include <memory_resource>
namespace Omni
{
#if OMNI_CLANG
	template<typename T>
	using PMRVector = std::experimental::fundamentals_v1::pmr::vector<T>;
#else
	template<typename T>
	using PMRVector = std::pmr::vector<T>;
#endif
}