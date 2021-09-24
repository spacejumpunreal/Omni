#include "BasePCH.h"
#include "MultiThread/SpinLock.h"
#include "Misc/AssertUtils.h"
#include "Misc/PlatformAPIs.h"

namespace Omni
{
	SpinLock::SpinLock()
		: mFlag(false)
	{}
	void SpinLock::Lock()
	{
		while (true)
		{
			bool expected = false;
			if (mFlag.compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
				return;
			PauseThread();
		}
	}
	bool SpinLock::TryLock()
	{
		bool expected = false;
		return mFlag.compare_exchange_strong(expected, true, std::memory_order_release, std::memory_order_relaxed);
	}
	void SpinLock::Unlock()
	{
		CheckDebug(mFlag.load(std::memory_order_relaxed), "can only unlock a locked lock");
		mFlag.store(false, std::memory_order_release);
	}
}

