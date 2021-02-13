#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Concurrency/IShared.h"
#include "Runtime/Platform/PlatformDefs.h"

namespace Omni
{
	class DispatchQueue
	{
	public:

	};
	class DispatchWorkItem : public IShared
	{
	public:
		template<typename T>
		static DispatchWorkItem& Create(void (*func)(T*), T* argPtr)
		{
			static_assert(std::is_standard_layout_v<T> && std::is_trivial_v<T>);
			static_assert(alignof(DispatchWorkItem) <= sizeof(void*));
			DispatchWorkItem& r = CreateImpl(func, sizeof(T));
			void* ap = GetArgPtr(&r);
			*(T*)ap = *argPtr;
			return r;
		}
		void Perform();
		void Destroy() override;
		DispatchWorkItem* GetNext();
	private:
		static DispatchWorkItem& CreateImpl(void* f, size_t aSize);
		FORCEINLINE static void* GetArgPtr(DispatchWorkItem* item)
		{
			return ((u8*)item) + sizeof(DispatchWorkItem);
		}
	private:
		void* mNext;
		void* mFPtr;
	};

	class DispatchGroup
	{
	public:
	};
}