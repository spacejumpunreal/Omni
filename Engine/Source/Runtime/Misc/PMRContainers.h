#pragma once
#include "Runtime/Omni.h"

#if OMNI_CLANG
#include <experimental/vector>
#include <experimental/unordered_map>
#include <experimental/unordered_set>
#else
#include <vector>
#include <unordered_map>
#include <unordered_set>
#endif

namespace Omni
{
#if OMNI_CLANG
	template<typename T>
	using PMRVector = std::experimental::pmr::vector<T>;

	template<typename K, typename V, typename Hasher, typename KeyEqual>
	using PMRUnorderedMap = std::experimental::pmr::unordered_map<K, V, Hasher, KeyEqual>;

    template<typename K, typename Hasher, typename KeyEqual>
    using PMRUnorderedSet = std::experimental::pmr::unordered_set<K, Hasher, KeyEqual>;
#else
	template<typename T>
	using PMRVector = std::pmr::vector<T>;

	template<typename K, typename V, typename Hasher, typename KeyEqual>
	using PMRUnorderedMap = std::pmr::unordered_map<K, V, Hasher, KeyEqual>;

    template<typename K, typename Hasher, typename KeyEqual>
    using PMRUnorderedSet = std::pmr::unordered_set<K, Hasher, KeyEqual>;
#endif
}
