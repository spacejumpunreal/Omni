#include "Runtime/Concurrency/SpinLock.h"
#include "Runtime/Misc/AssertUtils.h"
#include "Runtime/Platform/PlatformAPIs.h"

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
			if (mFlag.compare_exchange_strong(expected, true, std::memory_order::acquire, std::memory_order::memory_order_relaxed))
				return;
			Pause();
		}
	}
	bool SpinLock::TryLock()
	{
		bool expected = false;
		return mFlag.compare_exchange_strong(expected, true, std::memory_order::release, std::memory_order::memory_order_relaxed);
	}
	void SpinLock::Unlock()
	{
		CheckDebug(mFlag.load(std::memory_order::relaxed), "can only unlock a locked lock");
		mFlag.store(false);
	}
}