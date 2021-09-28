#include "CorePCH.h"
#include "Allocator/MemoryModule.h"
#include "Concurrency/JobPrimitives.h"
#include "Concurrency/ConcurrencyModule.h"
#include "Concurrency/ThreadUtils.h"
#include "Container/Queue.h"
#include "Misc/Padding.h"
#include "MultiThread/SpinLock.h"


namespace Omni
{
	using UnaryFunctionType = void (*)(void*);

	struct DispatchQueuePrivate
	{
	public:
		CacheAligned<SpinLock>					mQueueLock;
		CacheAligned<SpinLock>					mExecLock;
		Queue									mQueue;
		const char*								mName;
	public:
		DispatchQueuePrivate();
	};

	DispatchQueuePrivate::DispatchQueuePrivate()
		: mName(nullptr)
	{
	}
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
	static void PollDispatchQueueFunc(DispatchQueue** pq)
	{
		DispatchQueuePrivate* queue = (DispatchQueuePrivate*)*pq;
		if (!queue->mExecLock.Data.TryLock())
			return;

		bool isOwner = true;
		bool done = false;
		while (!done)
		{
			if (!isOwner)
				queue->mExecLock.Data.Lock();
			queue->mQueueLock.Data.Lock();
			DispatchWorkItem* head = static_cast<DispatchWorkItem*>(queue->mQueue.DequeueAll());
			queue->mQueueLock.Data.Unlock();
			while (head != nullptr)
			{
				head->Perform();
				DispatchWorkItem* sp = head;
				head = static_cast<DispatchWorkItem*>(head->Next);
				sp->Destroy();
			}
			queue->mExecLock.Data.Unlock();
			isOwner = false;
			queue->mQueueLock.Data.Lock();
			done = queue->mQueue.IsEmpty();
			queue->mQueueLock.Data.Unlock();
		}
	}
	void DispatchQueue::Enqueue(DispatchWorkItem* head, DispatchWorkItem* tail)
	{
#if OMNI_DEBUG
		{
			CheckAlways(tail->Next == nullptr);
			DispatchWorkItem* p = head, *pp = nullptr;
			while (p != nullptr)
			{
				pp = p;
				p = (DispatchWorkItem*)p->Next;
			}
			CheckAlways(pp == tail);
		}
#endif
		DispatchQueuePrivate& self = mData.Ref<DispatchQueuePrivate>();
		self.mQueueLock.Data.Lock();
		self.mQueue.Enqueue(head, tail);
		self.mQueueLock.Data.Unlock();
		DispatchQueue* pq = this;
		DispatchWorkItem& pollQueue = DispatchWorkItem::Create(PollDispatchQueueFunc, &pq);
		ConcurrencyModule::Get().Async(pollQueue);
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
	DispatchGroup::~DispatchGroup()
	{
		mData.DestroyAs<DispatchGroupPrivate>();
	}
	void DispatchGroup::Destroy()
	{
		this->~DispatchGroup(); //actually no need for dtor, pod, hope compiler can optimize it off
		PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::CacheLine);
		alloc.resource()->deallocate(this, 0);//note that we can use 0 because we use cacheline allocator
	}
	void DispatchGroup::Enter()
	{
		DispatchGroupPrivate& self = mData.Ref<DispatchGroupPrivate>();
#if OMNI_DEBUG
		CheckAlways(!self.mLocked.load(std::memory_order_relaxed));
#endif
		self.mEnterCount.fetch_add(1, std::memory_order_relaxed);
	}
	void DispatchGroup::Leave()
	{
		DispatchGroupPrivate& self = mData.Ref<DispatchGroupPrivate>();
#if OMNI_DEBUG
		if (self.mLocked.load(std::memory_order_relaxed))
			self.mLocked.store(true, std::memory_order_relaxed);
#endif
		size_t v = self.mEnterCount.fetch_sub(1, std::memory_order_release);
		if (v == 1 && self.mNext)
		{
			if (self.mQueue)
				self.mQueue->Enqueue(self.mNext, self.mNext);
			else
			{
				ConcurrencyModule::Get().Async(*self.mNext);
			}
		}
		if (v == 1)
			Destroy();
	}
	void DispatchGroup::Notify(DispatchWorkItem& item, DispatchQueue* queue)
	{
		DispatchGroupPrivate& self = mData.Ref<DispatchGroupPrivate>();
#if OMNI_DEBUG
		CheckAlways(!self.mLocked.load(std::memory_order_relaxed));
#endif
		self.mNext = &item;
		self.mQueue = queue;
	}

}

