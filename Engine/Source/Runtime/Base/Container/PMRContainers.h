#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Memory/StdMemoryResource.h"

#if OMNI_CLANG
#include <experimental/unordered_map>
#include <experimental/unordered_set>
#include <experimental/vector>
#else
#include <unordered_map>
#include <unordered_set>
#include <vector>
#endif

namespace Omni
{
	template<typename T>
	using PMRVector = StdPmr::vector<T>;

	template<typename K, typename V, typename Hasher, typename KeyEqual>
	using PMRUnorderedMap = StdPmr::unordered_map<K, V, Hasher, KeyEqual>;

    template<typename K, typename Hasher, typename KeyEqual>
    using PMRUnorderedSet = StdPmr::unordered_set<K, Hasher, KeyEqual>;
}
