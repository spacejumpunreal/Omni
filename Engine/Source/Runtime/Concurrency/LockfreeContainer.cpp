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
				CheckDebug(mColdCount > 0);
			}
			std::swap(mHotList, mColdList);
			std::swap(mHotCount, mColdCount);
		}
		--mHotCount;
		LockfreeNode* ret = mHotList;
		mHotList = mHotList->Next;
		return ret;
	}
	void LockfreeNodeCachePerThreadData::Free(LockfreeNode* node)
	{
		if (mHotCount >= LockfreeNodeTransferBatchCount)
		{
			if (mColdCount >= LockfreeNodeTransferBatchCount)
			{
				gLockfreeNodeCacheGlobalData.FreeBatch(mColdList, mColdCount);
				mColdCount = 0;
			}
			++mColdCount;
			node->Next = mColdList;
			mColdList = node;
		}
		++mHotCount;
		node->Next = mHotList;
		mHotList = node;
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
		lst->Data[1] = lst->Next;
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
				batch = batch->Next;
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
	void LockfreeNodeCache::TreadFinalize()
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
		} while (_InterlockedCompareExchange128((volatile long long*)&mHead, (long long)node, (long long)mHead.Tag + 1, (long long*)&oldHead) == 0);
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
		} while (_InterlockedCompareExchange128((volatile long long*)&mHead, (long long)(oldHead.Ptr->Next), (long long)oldHead.Tag, (long long*)&oldHead) == 0);
		return oldHead.Ptr;
#endif
	}
	LockfreeQueue::LockfreeQueue()
	{
		mHead.Tag = 0;
		mTail = mHead.Ptr = LockfreeNodeCache::Alloc();
		mHead.Ptr->Next = nullptr;
	}
	LockfreeQueue::~LockfreeQueue()
	{
		CheckAlways(mHead.Ptr->Next = nullptr);
		LockfreeNodeCache::Free(mHead.Ptr);
	}
	void LockfreeQueue::Enqueue(LockfreeNode* /*first*/, LockfreeNode* /*last*/)
	{
	}
	LockfreeNode* LockfreeQueue::Dequeue()
	{
		return nullptr;
	}
}