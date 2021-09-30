#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Prelude/PlatformDefs.h"
#include "Runtime/Prelude/SuppressWarning.h"
#include <type_traits>


namespace Omni
{

	template<typename T, size_t TSize = sizeof(T)>
	struct PrivateDataType
	{
		using Type = T;
		static constexpr size_t Size = TSize;
	};
	static constexpr u32 PrivateDataValidMask = 0x12345678u;
	static constexpr u32 PrivateDataDestroyedMask = 0xdeadbeef;

	template<size_t Size, size_t Align = OMNI_DEFAULT_ALIGNMENT, bool CheckDestroy = false>
	struct alignas(Align) PrivateData
	{
		template<typename T, size_t = sizeof(T)>
		FORCEINLINE T* Ptr() { return (T*)RawData; }

		template<typename T, size_t = sizeof(T)>
		FORCEINLINE T& Ref() { return *(T*)RawData; }

		template<typename T>
		FORCEINLINE u32& CheckMaskRef() { return &RawData[AlignedWords - 1]; }

		template<typename T, typename... Args>
		FORCEINLINE PrivateData(PrivateDataType<T>, Args&&... args)
		{
			static_assert(sizeof(T) <= Size, "sizeof(T) <= Size");
			static_assert(alignof(T) <= Align, "alignof(T) <= Align");
			T* p = Ptr<T>();
			new (p) T(std::forward<Args>(args)...);
			if constexpr (CheckDestroy)
			{
				CheckMaskRef() = PrivateDataValidMask;
			}
		}
		template<typename T, size_t = sizeof(T)>
		FORCEINLINE void DestroyAs()
		{
			if constexpr (CheckDestroy)
			{
				CheckDebug(CheckMaskRef() == PrivateDataValidMask);
				CheckMaskRef() = PrivateDataDestroyedMask;
			}
			
			static_assert(sizeof(T) <= Size, "sizeof(T) <= Size");
			static_assert(alignof(T) <= Align, "alignof(T) <= Align");
			T* p = (T*)RawData;
			p->~T();
		}
		~PrivateData()
		{
			if constexpr (CheckDestroy)
			{
				CheckAlways(CheckMaskRef() == PrivateDataDestroyedMask);
			}
		}
	public:
		static_assert(Align >= (sizeof(int)), "Align size should be at least sizeof(int)");
		static constexpr size_t CheckMaskSize = sizeof(u32);
		static constexpr size_t AlignedSize = (Size + CheckMaskSize + Align - 1) / Align * Align;
		static constexpr size_t AlignedWords = AlignedSize / sizeof(u32);
	private:
		u32 RawData[AlignedWords];
	};


}

