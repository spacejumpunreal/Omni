#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Base/Container/LinkedList.h"
#include "Runtime/Base/Misc/PrivateData.h"
#include "Runtime/Base/MultiThread/IShared.h"
#include "Runtime/Prelude/PlatformDefs.h"

namespace Omni
{
	class DispatchWorkItem;

	class DispatchQueue
	{
	public:
		CORE_API DispatchQueue();
		CORE_API ~DispatchQueue();
		CORE_API void SetName(const char* name);
		CORE_API void Enqueue(DispatchWorkItem* head, DispatchWorkItem* tail);
	private:
		PrivateData<CPU_CACHE_LINE_SIZE * 3, CPU_CACHE_LINE_SIZE>	mData;
	};

	class DispatchWorkItem : public SListNode
	{
	public:
		template<typename T>
		static DispatchWorkItem& Create(void (*func)(T*), T* argPtr)
		{
			static_assert(std::is_standard_layout_v<T> && std::is_trivial_v<T>);
			static_assert(alignof(DispatchWorkItem) <= sizeof(void*));
			DispatchWorkItem& r = CreatePrivate((void*)func, sizeof(T));
			if (argPtr)
			{
				void* ap = GetArgPtr(&r);
				*(T*)ap = *argPtr;
			}
			return r;
		}
		static DispatchWorkItem& Create(void (*func)())
		{
			static_assert(alignof(DispatchWorkItem) <= sizeof(void*));
			DispatchWorkItem& r = CreatePrivate((void*)func, 0);
			return r;
		}
		CORE_API void Perform();
		CORE_API void Destroy();
	private:
		CORE_API static DispatchWorkItem& CreatePrivate(void* f, size_t aSize);
		FORCEINLINE static void* GetArgPtr(DispatchWorkItem* item)
		{
			return ((u8*)item) + sizeof(DispatchWorkItem);
		}
		CORE_API DispatchWorkItem(void* fptr);
	private:
		void*				mFPtr;
	};

	class DispatchGroup
	{
	public:
		CORE_API static DispatchGroup& Create(size_t enterCount);
		CORE_API ~DispatchGroup();
		CORE_API void Destroy();
		CORE_API void Enter();
		CORE_API void Leave();
		CORE_API void Notify(DispatchWorkItem& item, DispatchQueue* queue);
	private:
		CORE_API DispatchGroup(size_t enterCount);
		PrivateData<CPU_CACHE_LINE_SIZE>	mData;
	};
}
