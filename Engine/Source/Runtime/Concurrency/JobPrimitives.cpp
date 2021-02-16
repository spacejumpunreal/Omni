#include "Runtime/Concurrency/JobPrimitives.h"
#include "Runtime/Concurrency/ConcurrencyModule.h"
#include "Runtime/Concurrency/SpinLock.h"
#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Misc/ContainerUtils.h"
#include "Runtime/Misc/Padding.h"


namespace Omni
{
	using UnaryFunctionType = void (*)(void*);

	struct DispatchQueuePrivate
	{
	public:
		CacheAlign<SpinLock>	mLock;
		Queue					mQueue;
		const char*				mName;
	public:
	};
	DispatchQueue::DispatchQueue()
		: mData(PrivateDataType<DispatchQueuePrivate>{})
	{
	}
	DispatchQueue::~DispatchQueue()
	{
		mData.DestroyAs<DispatchQueuePrivate>();
	}
	void DispatchQueue::SetName(const char* name)
	{
		mData.Ref<DispatchQueuePrivate>().mName = name;
	}
	void DispatchQueue::Enqueue(DispatchWorkItem* head, DispatchWorkItem* tail)
	{
		DispatchQueuePrivate& self = mData.Ref<DispatchQueuePrivate>();
		self.mLock.Data.Lock();
		self.mQueue.Enqueue(head, tail);
		self.mLock.Data.Unlock();
	}
	DispatchWorkItem* DispatchQueue::Dequeue()
	{
		DispatchQueuePrivate& self = mData.Ref<DispatchQueuePrivate>();
		self.mLock.Data.Lock();
		DispatchWorkItem* ret = static_cast<DispatchWorkItem*>(self.mQueue.Dequeue());
		self.mLock.Data.Unlock();
		return ret;
	}
	void DispatchWorkItem::Perform()
	{
		auto f = (UnaryFunctionType)mFPtr;
		void* ap = GetArgPtr(this);
		f(ap);
		mFPtr = nullptr;
	}
	void DispatchWorkItem::Destroy()
	{
		this->~DispatchWorkItem();
		PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::CacheLine);
		alloc.resource()->deallocate(this, 0);
	}
	DispatchWorkItem& DispatchWorkItem::CreatePrivate(void* f, size_t aSize)
	{
		PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::CacheLine);
		DispatchWorkItem* ret = (DispatchWorkItem*)alloc.resource()->allocate(sizeof(DispatchWorkItem) + aSize);
		new (ret)DispatchWorkItem(f);
		return *ret;
	}
	DispatchWorkItem::DispatchWorkItem(void* fptr)
		: SListNode(nullptr)
		, mFPtr(fptr)
	{}

	struct DispatchGroupPrivate
	{
		std::atomic<size_t>				mEnterCount;
		DispatchWorkItem*				mNext;
		DispatchQueue*					mQueue;
#if OMNI_DEBUG
		std::atomic<bool>		mLocked;
#endif
		DispatchGroupPrivate(size_t enterCount)
			: mEnterCount(enterCount)
			, mNext(nullptr)
		{
#if OMNI_DEBUG
			mLocked = false;
#endif
		}
	};
	DispatchGroup& DispatchGroup::Create(size_t enterCount)
	{
		PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::CacheLine);
		DispatchGroup* ret = (DispatchGroup*)alloc.resource()->allocate(sizeof(DispatchGroup));
		new (ret)DispatchGroup(enterCount);
		return *ret;
	}
	DispatchGroup::DispatchGroup(size_t enterCount)
		: mData(PrivateDataType<DispatchGroupPrivate>{}, enterCount)
	{
	}
	void DispatchGroup::Destroy()
	{
		this->~DispatchGroup(); //actually no need for dtor, pod, hope compiler can optimize it off
		PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::CacheLine);
		alloc.resource()->deallocate(this, 0);
	}
	void DispatchGroup::Enter()
	{
		DispatchGroupPrivate& self = mData.Ref<DispatchGroupPrivate>();
		CheckDebug(!self.mLocked.load(std::memory_order::relaxed));
		self.mEnterCount.fetch_add(1, std::memory_order::relaxed);
	}
	void DispatchGroup::Leave()
	{
		DispatchGroupPrivate& self = mData.Ref<DispatchGroupPrivate>();
#if OMNI_DEBUG
		if (self.mLocked.load(std::memory_order::relaxed))
			self.mLocked.store(true, std::memory_order::relaxed);
#endif
		size_t v = self.mEnterCount.fetch_sub(1, std::memory_order::release);
		if (v == 1 && self.mNext)
		{
			if (self.mQueue)
				self.mQueue->Enqueue(self.mNext, self.mNext);
			else
			{
				ConcurrencyModule::Get().Async(*self.mNext);
			}
		}
	}
	void DispatchGroup::Notify(DispatchWorkItem& item, DispatchQueue* queue)
	{
		DispatchGroupPrivate& self = mData.Ref<DispatchGroupPrivate>();
		CheckDebug(!self.mLocked.load(std::memory_order::relaxed));
		self.mNext = &item;
		self.mQueue = queue;
	}

}