#pragma once
#include <type_traits>

namespace Omni
{
	template<typename TInterface, typename TData>
	struct PImplCombine : public TInterface, public TData
	{
		static_assert(!std::is_polymorphic_v<TData>, "TData should have no vtable");
		struct Detail
		{
			template<typename T>
			static const T* OffsetPointer(const void* p, ptrdiff_t diff)
			{
				return (T*)((char*)p + diff);
			}
			template<typename T>
			static T* OffsetPointer(void* p, ptrdiff_t diff)
			{
				return (T*)((char*)p + diff);
			}
			static const ptrdiff_t Combine2Data = (ptrdiff_t)static_cast<TData*>((PImplCombine*)(0));
			static const ptrdiff_t Combine2Interface = (ptrdiff_t)static_cast<TInterface*>((PImplCombine*)(0));
		};

		static PImplCombine* GetCombinePtr(TInterface* tp) { return Detail::template OffsetPointer<PImplCombine>(tp, -Detail::Combine2Data); }
		static const PImplCombine* GetCombinePtr(const TInterface* tp) { return Detail::template OffsetPointer<PImplCombine>(tp, -Detail::Combine2Data); }

		static TData* GetData(TInterface* api) { return GetCombinePtr(api); }
		static const TData* GetData(const TInterface* api) { return GetCombinePtr(api); }
	};
}