#include "Runtime/Base/BasePCH.h"
#include "Runtime/Base/MultiThread/SpinLock.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Misc/PlatformAPIs.h"

namespace Omni
{
	SpinLock::SpinLock()
		: mData(false)
	{}
	void SpinLock::Lock()
	{
		while (true)
		{
			bool expected = false;
			if (mData.compare_exchange_strong(expected, true, std::memory_order_acquire, std::memory_order_relaxed))
				return;
			PauseThread();
		}
	}
	bool SpinLock::TryLock()
	{
		bool expected = false;
		return mData.compare_exchange_strong(expected, true, std::memory_order_release, std::memory_order_relaxed);
	}
	void SpinLock::Unlock()
	{
		CheckDebug(mData.load(std::memory_order_relaxed), "can only unlock a locked lock");
		mData.store(false, std::memory_order_release);
	}
}

