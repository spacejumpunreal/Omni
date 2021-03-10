#pragma once
#include "Runtime/Omni.h"
namespace Omni
{
	void TestLockfreeStackSingleThread();
	void TestLockfreeQueueSingleThread();
	void TestLockfreeQueueMultiThread();
	void TestLockfreeStackMultiThread();
	void TestPMRAllocate();
	void TestScratchStack();
	void TestSpinLock();
	void TestMultiThreadAllocation();
	void TestDispatchQueue();
	void TestAsync();

	FORCEINLINE void TestAll()
	{
		TestLockfreeStackSingleThread();
		TestLockfreeQueueSingleThread();
		TestLockfreeStackMultiThread();
		TestLockfreeQueueMultiThread();

		TestPMRAllocate();
		TestScratchStack();
		TestSpinLock();
		TestMultiThreadAllocation();
		TestDispatchQueue();
		TestAsync();
	}
}