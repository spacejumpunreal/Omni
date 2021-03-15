#pragma once
#include <type_traits>

namespace Omni
{
	template<typename TInterface, typename TData>
	struct PImplCombine final : public TInterface, public TData
	{
		static_assert(!std::is_polymorphic_v<TData>, "TData should have no vtable");
		static PImplCombine* GetCombinePtr(TInterface* tp) 
		{ 
			PImplCombine* p = (PImplCombine*)0x10000;
			ptrdiff_t diff = ((u8*)p) - ((u8*)static_cast<TInterface*>(p));
			return (PImplCombine*)((u8*)tp + diff);
		}
		static const PImplCombine* GetCombinePtr(const TInterface* tp)
		{
			PImplCombine* p = (PImplCombine*)0x10000;
			ptrdiff_t diff = ((u8*)p) - ((u8*)static_cast<TInterface*>(p));
			return (const PImplCombine*)((u8*)tp + diff);
		}
		static PImplCombine* GetCombinePtr(TData* tp)
		{
			PImplCombine* p = (PImplCombine*)0x10000;
			ptrdiff_t diff = ((u8*)p) - ((u8*)static_cast<TData*>(p));
			return (PImplCombine*)((u8*)tp + diff);
		}

		static TData* GetData(TInterface* api) { return GetCombinePtr(api); }
		static const TData* GetData(const TInterface* api) { return GetCombinePtr(api); }


		template<typename... Args>
		PImplCombine(Args&&... args)
			: TData{ std::forward<Args>(args)... }
		{}

	};
}

