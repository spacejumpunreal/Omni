#pragma once
#include "Runtime/Omni.h"
namespace Omni
{
	void TestLockfreeStackSingleThread();
	void TestLockfreeQueueSingleThread();
	void TestLockfreeQueueMultiThread();
	void TestPMRAllocate();
	void TestScratchStack();
	void TestSpinLock();
	void TestMultiThreadAllocation();
	void TestDispatchQueue();
	void TestAsync();
}