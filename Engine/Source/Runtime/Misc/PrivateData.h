#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Misc/AssertUtils.h"

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
	template<size_t Size, size_t Align = OMNI_DEFAULT_ALIGNMENT>
	struct alignas(Align) PrivateData
	{
		template<typename T, size_t = sizeof(T)>
		FORCEINLINE T* Ptr() { return (T*)RawData; }

		template<typename T, size_t = sizeof(T)>
		FORCEINLINE T& Ref() { return *(T*)RawData; }

		template<typename T, typename... Args>
		FORCEINLINE PrivateData(PrivateDataType<T>, Args&&... args)
			: mDestroyed(false)
		{
			static_assert(sizeof(T) <= Size, "sizeof(T) <= Size");
			T* p = Ptr<T>();
			new (p) T(std::forward<Args>(args)...);
		}
		template<typename T, size_t = sizeof(T)>
		FORCEINLINE void DestroyAs()
		{
			CheckDebug(!mDestroyed);
			static_assert(sizeof(T) <= Size, "sizeof(T) <= Size");
			T* p = (T*)RawData;
			p->~T();
			mDestroyed = true;
		}
		~PrivateData()
		{
			CheckDebug(mDestroyed);
		}
	public:
		static_assert(Align >= (sizeof(int)), "Align size should be at least sizeof(int)");
		static constexpr size_t AlignedSize = (Size + Align - 1) / Align * Align;
		static constexpr size_t AlignedWords = AlignedSize / sizeof(int);
	private:
		int RawData[AlignedWords];
		bool mDestroyed;
	};
#pragma warning( pop )
}