#include "Runtime/Concurrency/LockfreeContainer.h"
#include "Runtime/Concurrency/IThreadLocal.h"
#include "Runtime/Math/CompileTime.h"
#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Test/AssertUtils.h"
#include <unordered_set>
#include <tuple>

namespace Omni
{
	constexpr u32 LockfreeNodeMmapSize = 64 * 1024;
	constexpr u32 LockfreeNodeTransferBatchCount = 32;
	//cache managing policy:
	class LockfreeNodeCachePerThreadData
	{
	public:
		LockfreeNodeCachePerThreadData();
		LockfreeNode* Allocate();
		void Free(LockfreeNode* node);
		void Cleanup();
		bool IsClean();
	private:
		u32					mHotCount;
		u32					mColdCount;
		LockfreeNode*		mHotList;
		LockfreeNode*		mColdList;
	};

	class LockfreeNodeCacheGlobalData
	{
	public:
		LockfreeNodeCacheGlobalData();
		std::tuple<LockfreeNode*, u32> AllocateBatch();
		void FreeBatch(LockfreeNode* lst, u32 count);
		void Cleanup();
		bool IsClean();
	private:
		LockfreeStack		mBatchStack;
	};

	//global
	OMNI_DECLARE_THREAD_LOCAL(LockfreeNodeCachePerThreadData, gLockfreeNodeCachePerThreadData);
	static LockfreeNodeCacheGlobalData gLockfreeNodeCacheGlobalData;


	LockfreeNodeCachePerThreadData::LockfreeNodeCachePerThreadData()
		: mHotCount(0)
		, mColdCount(0)
		, mHotList(nullptr)
		, mColdList(nullptr)
	{
	}
	LockfreeNode* LockfreeNodeCachePerThreadData::Allocate()
	{
		if (mHotCount == 0)
		{
			if (mColdCount == 0)
			{
				std::tie(mColdList, mColdCount) = gLockfreeNodeCacheGlobalData.AllocateBatch();
			}
			std::swap(mHotList, mColdList);
			std::swap(mHotCount, mColdCount);
		}
		--mHotCount;
		LockfreeNode* ret = mHotList;
		mHotList = (LockfreeNode*)mHotList->Next;
		ret->Next = nullptr;
		return ret;
	}
	void LockfreeNodeCachePerThreadData::Free(LockfreeNode* node)
	{
		if (mHotCount == LockfreeNodeTransferBatchCount)
		{
			++mColdCount;
			node->Next = mColdList;
			mColdList = node;
			if (mColdCount == LockfreeNodeTransferBatchCount)
			{
				gLockfreeNodeCacheGlobalData.FreeBatch(mColdList, mColdCount);
				mColdCount = 0;
				mColdList = nullptr;
			}
		}
		else
		{
			++mHotCount;
			node->Next = mHotList;
			mHotList = node;
		}
		
	}
	void LockfreeNodeCachePerThreadData::Cleanup()
	{
		if (mHotCount > 0)
			gLockfreeNodeCacheGlobalData.FreeBatch(mHotList, mHotCount);
		if (mColdCount > 0)
			gLockfreeNodeCacheGlobalData.FreeBatch(mColdList, mColdCount);
		mHotCount = 0;
		mColdCount = 0;
		mHotList = nullptr;
		mColdList = nullptr;
	}
	bool LockfreeNodeCachePerThreadData::IsClean()
	{
		return mHotCount == 0 && mColdCount == 0 && mHotList == nullptr && mColdList == nullptr;
	}
	LockfreeNodeCacheGlobalData::LockfreeNodeCacheGlobalData()
	{
		CheckAlways(mBatchStack.Pop() == nullptr);
	}
	std::tuple<LockfreeNode*,u32> LockfreeNodeCacheGlobalData::AllocateBatch()
	{
		LockfreeNode* n = mBatchStack.Pop();
		if (n == nullptr)
		{
			constexpr u32 totalCount = LockfreeNodeMmapSize / sizeof(LockfreeNode);
			LockfreeNode* p = (LockfreeNode*)MemoryModule::Get().Mmap(LockfreeNodeMmapSize);
			constexpr u32 batch = totalCount / LockfreeNodeTransferBatchCount;
			LockfreeNode* tp = p;
			for (u32 iBatch = 0; iBatch < batch; ++iBatch)
			{
				for (u32 iNode = 0; iNode < LockfreeNodeTransferBatchCount; ++iNode)
					tp[iNode].Next = &tp[iNode + 1];
				tp[LockfreeNodeTransferBatchCount - 1].Next = nullptr;
				if (iBatch != 0)
					FreeBatch(tp, LockfreeNodeTransferBatchCount);
				tp = tp + (u64)LockfreeNodeTransferBatchCount;
			}
			return std::make_tuple(p, LockfreeNodeTransferBatchCount);
		}
		else
		{
			n->Next = (LockfreeNode*)n->Data[1];
			return std::make_tuple(n, (u32)(u64)n->Data[0]);
		}
	}
	void LockfreeNodeCacheGlobalData::FreeBatch(LockfreeNode* lst, u32 count)
	{
		lst->Data[0] = (void*)(u64)count;
		lst->Data[1] = (LockfreeNode*)lst->Next;
		mBatchStack.Push(lst);
	}
	bool LockfreeNodeCacheGlobalData::IsClean()
	{
		return mBatchStack.Pop() == nullptr;
	}
	void LockfreeNodeCacheGlobalData::Cleanup()
	{
		std::pmr::unordered_set<u64> pages;
		while (true)
		{
			LockfreeNode* batch = mBatchStack.Pop();
			if (!batch)
				break;
			batch->Next = (LockfreeNode*)batch->Data[1];
			while (batch)
			{
				u64 addr = (u64)batch;
				addr >>= CompileTimeLog2(LockfreeNodeMmapSize);
				pages.insert(addr);
				batch = (LockfreeNode*)batch->Next;
			}
		}
		for (u64 addr : pages)
		{
			MemoryModule::Get().Munmap((void*)(addr << CompileTimeLog2(LockfreeNodeMmapSize)), LockfreeNodeMmapSize);
		}
	}
	void LockfreeNodeCache::GlobalInitialize()
	{
		CheckAlways(gLockfreeNodeCacheGlobalData.IsClean());
	}
	void LockfreeNodeCache::GlobalFinalize()
	{
		gLockfreeNodeCacheGlobalData.Cleanup();
	}
	void LockfreeNodeCache::ThreadInitialize()
	{
		CheckAlways(gLockfreeNodeCachePerThreadData.IsClean());
	}
	void LockfreeNodeCache::ThreadFinalize()
	{
		gLockfreeNodeCachePerThreadData.GetRaw().Cleanup();
	}
	LockfreeNode* LockfreeNodeCache::Alloc()
	{
		return gLockfreeNodeCachePerThreadData.GetRaw().Allocate();
	}
	void LockfreeNodeCache::Free(LockfreeNode* node)
	{
		gLockfreeNodeCachePerThreadData.GetRaw().Free(node);
	}
	LockfreeStack::LockfreeStack()
	{
		mHead.Tag = 0;
		mHead.Ptr = nullptr;
	}
	LockfreeStack::~LockfreeStack()
	{
		CheckDebug(mHead.Ptr == nullptr);
	}

	void LockfreeStack::Push(LockfreeNode* node)
	{
#if OMNI_WINDOWS
		TaggedPointer oldHead;
		do
		{
			oldHead.Tag = mHead.Tag;
			oldHead.Ptr = mHead.Ptr;
			node->Next = oldHead.Ptr;
		} while (_InterlockedCompareExchange128((volatile long long*)&mHead, (long long)node, (long long)oldHead.Tag + 1, (long long*)&oldHead) == 0);
#endif
	}
	LockfreeNode* LockfreeStack::Pop()
	{
#if OMNI_WINDOWS
		TaggedPointer oldHead;
		do
		{
			oldHead.Tag = mHead.Tag;
			oldHead.Ptr = mHead.Ptr;
			if (oldHead.Ptr == nullptr)
				return nullptr;
		} while (_InterlockedCompareExchange128((volatile long long*)&mHead, (long long)(oldHead.Ptr->Next), (long long)oldHead.Tag + 1, (long long*)&oldHead) == 0);
		oldHead.Ptr->Next = nullptr;
		return oldHead.Ptr;
#endif
	}
	template<u32 NodeDataCount>
	LockfreeQueue<NodeDataCount>::LockfreeQueue()
	{
		mHead.Tag = 0;
		mTail = mHead.Ptr = LockfreeNodeCache::Alloc();
		mHead.Ptr->Next = nullptr;
	}
	template<u32 NodeDataCount>
	LockfreeQueue<NodeDataCount>::~LockfreeQueue()
	{
		CheckAlways(mHead.Ptr->Next == nullptr);
		LockfreeNodeCache::Free(mHead.Ptr);
	}
	template<u32 NodeDataCount>
	void LockfreeQueue<NodeDataCount>::Enqueue(LockfreeNode* first, LockfreeNode* last)
	{
		last->Next = nullptr;
		LockfreeNode* oldTail = std::atomic_exchange_explicit((std::atomic<LockfreeNode*>*)&mTail, last, std::memory_order_release);
		std::atomic_store_explicit((std::atomic<LockfreeNode*>*)&oldTail->Next, first, std::memory_order_release);
	}
	template<u32 NodeDataCount>
	LockfreeNode* LockfreeQueue<NodeDataCount>::Dequeue()
	{
#if OMNI_WINDOWS
		TaggedPointer oldHead;
		LockfreeNode* next;
		void* sData[LockfreeNode::MaxDataSlots];
		oldHead.Tag = mHead.Tag;
		oldHead.Ptr = mHead.Ptr;
		do
		{
			next = std::atomic_load_explicit((std::atomic<LockfreeNode*>*)&(oldHead.Ptr->Next), std::memory_order_acquire);
			if (next == nullptr)
				return nullptr;
			for (u32 i = 0; i < NodeDataCount; ++i)
				sData[i] = next->Data[i];
		} while (_InterlockedCompareExchange128((volatile long long*)&mHead, (long long)next, (long long)oldHead.Tag + 1, (long long*)&oldHead) == 0);
		for (u32 i = 0; i < NodeDataCount; ++i)
			oldHead.Ptr->Data[i] = sData[i];
		oldHead.Ptr->Next = nullptr;
		return oldHead.Ptr;
#endif		
	}

	template class LockfreeQueue<1>;
	template class LockfreeQueue<2>;
	template class LockfreeQueue<3>;
	template class LockfreeQueue<4>;
	template class LockfreeQueue<5>;
	template class LockfreeQueue<6>;
	template class LockfreeQueue<7>;
}
