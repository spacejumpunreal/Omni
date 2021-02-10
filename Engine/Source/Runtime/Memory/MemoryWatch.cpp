#include "Runtime/Memory/MemoryWatch.h"

namespace Omni
{
	void MemoryWatch::Dump(MemoryStats& stats)
	{
		stats.Used = mUsed.load(std::memory_order::memory_order_relaxed);
		stats.Peak = mPeak.load(std::memory_order::memory_order_relaxed);
		stats.Total = mTotal.load(std::memory_order::memory_order_relaxed);
		stats.Throughput = mThroughput.load(std::memory_order::memory_order_relaxed);
	}
}