#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/Concurrency/JobPrimitives.h"
#include "Runtime/Base/Misc/Padding.h"
#include "Runtime/Base/MultiThread/LockQueue.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Concurrency/ConcurrencyModule.h"
#include "Runtime/Core/Concurrency/ThreadUtils.h"


namespace Omni
{
	using UnaryFunctionType = void (*)(void*);
	//DispatchWorkItem impl
	void DispatchWorkItem::Perform()
	{
		auto f = (UnaryFunctionType)mFPtr;
		void* ap = GetArgPtr(this);
		f(ap);
		mFPtr = nullptr;
	}

	void DispatchWorkItem::Destroy()
	{
		PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(mMemKind);
		this->~DispatchWorkItem();
		alloc.resource()->deallocate(this, 0);
	}

	DispatchWorkItem& DispatchWorkItem::CreatePrivate(void* f, size_t aSize, MemoryKind memKind)
	{
		PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(memKind);
		DispatchWorkItem* ret = (DispatchWorkItem*)alloc.resource()->allocate(sizeof(DispatchWorkItem) + aSize);
		new (ret)DispatchWorkItem(f, memKind);
		return *ret;
	}

	DispatchWorkItem::DispatchWorkItem(void* fptr, MemoryKind memKind)
		: SListNode(nullptr)
		, mFPtr(fptr)
		, mMemKind(memKind)
	{}


	struct DispatchGroupPrivate
	{
		std::atomic<size_t>				mEnterCount;
		DispatchWorkItem*				mNotifyTask;
		LockQueue*						mNotifyQueue; //if speicified, mNotifyTask will execute on this queue
#if OMNI_DEBUG
		std::atomic<bool>		mLocked;
#endif
		DispatchGroupPrivate(size_t enterCount)
			: mEnterCount(enterCount)
			, mNotifyTask(nullptr)
		{
#if OMNI_DEBUG
			mLocked = false;
#endif
		}
	};

	//DispatchGroup iml
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

#if OMNI_DEBUG
	void DispatchGroup::Lock()
	{
		DispatchGroupPrivate& self = mData.Ref<DispatchGroupPrivate>();
		self.mLocked.store(true, std::memory_order_relaxed);
	}
#endif

	void DispatchGroup::Leave()
	{
		DispatchGroupPrivate& self = mData.Ref<DispatchGroupPrivate>();
#if OMNI_DEBUG
		self.mLocked.store(true, std::memory_order_relaxed);
#endif
		size_t v = self.mEnterCount.fetch_sub(1, std::memory_order_release);
		if (v == 1 && self.mNotifyTask)
		{
			self.mNotifyQueue->Enqueue(self.mNotifyTask);
		}
		if (v == 1)
			Destroy();
	}

	void DispatchGroup::Notify(DispatchWorkItem& item, LockQueue* queue)
	{
		DispatchGroupPrivate& self = mData.Ref<DispatchGroupPrivate>();
#if OMNI_DEBUG
		CheckAlways(!self.mLocked.load(std::memory_order_relaxed));
#endif
		CheckAlways((self.mNotifyTask && self.mNotifyQueue) || (self.mNotifyTask == nullptr && self.mNotifyQueue == nullptr));
		self.mNotifyTask = &item;
		self.mNotifyQueue = queue;
	}
}

