#pragma once
#include "Runtime/Omni.h"

#if OMNI_CLANG
#include <experimental/vector>
#else
#include <vector>
#include <unordered_map>
#endif

namespace Omni
{
#if OMNI_CLANG
	template<typename T>
	using PMRVector = std::experimental::pmr::vector<T>;

	template<typename K, typename V, typename Hasher, typename KeyEqual>
	using PMRUnorderedMap = std::experimental::pmr::unordered_map<K, V, Hasher, KeyEqual>;
#else
	template<typename T>
	using PMRVector = std::pmr::vector<T>;

	template<typename K, typename V, typename Hasher, typename KeyEqual>
	using PMRUnorderedMap = std::pmr::unordered_map<K, V, Hasher, KeyEqual>;
#endif
}
