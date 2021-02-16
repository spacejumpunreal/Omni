#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Concurrency/IShared.h"
#include "Runtime/Misc/LinkedListUtils.h"
#include "Runtime/Misc/PrivateData.h"
#include "Runtime/Platform/PlatformDefs.h"

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
		DispatchWorkItem* Dequeue();
	private:
		PrivateData<CPU_CACHE_LINE_SIZE * 2, CPU_CACHE_LINE_SIZE>	mData;
	};

	class DispatchWorkItem : public SListNode
	{
	public:
		template<typename T = void>
		static DispatchWorkItem& Create(void (*func)(T*), T* argPtr)
		{
			static_assert(std::is_standard_layout_v<T> && std::is_trivial_v<T>);
			static_assert(alignof(DispatchWorkItem) <= sizeof(void*));
			DispatchWorkItem& r = CreatePrivate(func, sizeof(T));
			if (argPtr)
			{
				void* ap = GetArgPtr(&r);
				*(T*)ap = *argPtr;
			}
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

	class DispatchGroup : IShared
	{
	public:
		static DispatchGroup& Create(size_t enterCount);
		//~DispatchGroup(); //no need for dtor, pod
		void Destroy() override;
		void Enter();
		void Leave();
		void Notify(DispatchWorkItem& item, DispatchQueue* queue);
	private:
		DispatchGroup(size_t enterCount);
		PrivateData<CPU_CACHE_LINE_SIZE>	mData;
	};
}