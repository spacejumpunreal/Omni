#pragma once
#include "Omni.h"
#include "MultiThread/IShared.h"
#include "Container/LinkedList.h"
#include "Misc/PrivateData.h"
#include "PlatformDefs.h"

namespace Omni
{
	class DispatchWorkItem;

	class DispatchQueue
	{
	public:
		DispatchQueue();
		~DispatchQueue();
		void SetName(const char* name);
		void Enqueue(DispatchWorkItem* head, DispatchWorkItem* tail);
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
		void Perform();
		void Destroy();
	private:
		static DispatchWorkItem& CreatePrivate(void* f, size_t aSize);
		FORCEINLINE static void* GetArgPtr(DispatchWorkItem* item)
		{
			return ((u8*)item) + sizeof(DispatchWorkItem);
		}
		DispatchWorkItem(void* fptr);
	private:
		void*				mFPtr;
	};

	class DispatchGroup
	{
	public:
		static DispatchGroup& Create(size_t enterCount);
		~DispatchGroup();
		void Destroy();
		void Enter();
		void Leave();
		void Notify(DispatchWorkItem& item, DispatchQueue* queue);
	private:
		DispatchGroup(size_t enterCount);
		PrivateData<CPU_CACHE_LINE_SIZE>	mData;
	};
}
