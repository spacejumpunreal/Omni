#pragma once
#include "Runtime/Omni.h"

namespace Omni
{
	template<typename T, size_t TSize = sizeof(T)>
	struct PrivateDataType
	{
		using Type = T;
		static constexpr size_t Size = TSize;
	};

#pragma warning ( push )
#pragma warning( disable : 4324)
	template<size_t Size, size_t Align=OMNI_DEFAULT_ALIGNMENT>
	struct alignas(Align) PrivateData
	{
		template<typename T>
		FORCEINLINE T* Ptr() { return (T*)RawData; }

		template<typename T>
		FORCEINLINE T& Ref() { return *(T*)RawData; }

		template<typename T, typename... Args>
		FORCEINLINE PrivateData(PrivateDataType<T>, Args&&... args)
		{
			static_assert(sizeof(T) <= Size, "sizeof(T) <= Size");
			T* p = Ptr<T>();
			new (p) T(std::forward<Args>(args)...);
		}
		int RawData[(Size + Align - 1) / Align];
	};
#pragma warning( pop )
}