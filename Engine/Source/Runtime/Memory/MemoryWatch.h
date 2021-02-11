#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Memory/MemoryDefs.h"
#include <atomic>

namespace Omni
{
	constexpr static bool WatchMemoryUsed = true || OMNI_DEBUG; //some allocator need this to check leak
	constexpr static bool WatchMemoryPeek = true;
	constexpr static bool WatchMemoryThroughput = true;

	class MemoryWatch
	{
	public:
		FORCEINLINE void Add(size_t size);
		FORCEINLINE void Sub(size_t size);
		void Dump(MemoryStats& stats);
	private:
		std::atomic<size_t>		mUsed;
		std::atomic<size_t>		mPeak;
		std::atomic<size_t>		mTotal;
		std::atomic<size_t>		mThroughput;
	};

	void MemoryWatch::Add(size_t size)
	{
		if constexpr (WatchMemoryUsed)
		{
			mUsed.fetch_add(size, std::memory_order::memory_order_relaxed);
		}
		if constexpr (WatchMemoryPeek)
		{
			size_t old;
			do
			{
				old = mPeak.load(std::memory_order::memory_order_relaxed);
			} while (!mPeak.compare_exchange_weak(old, old + size, std::memory_order::memory_order_relaxed));
		}
		if constexpr (WatchMemoryThroughput)
		{
			mThroughput.fetch_add(size, std::memory_order::memory_order_relaxed);
		}
	}
	void MemoryWatch::Sub(size_t size)
	{
		if constexpr (WatchMemoryUsed)
		{
			mUsed.fetch_sub(size, std::memory_order::memory_order_relaxed);
		}
	}
}