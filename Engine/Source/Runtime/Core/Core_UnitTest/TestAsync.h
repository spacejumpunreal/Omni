#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Misc/PlatformAPIs.h"
#include "Runtime/Core/Concurrency/ConcurrencyModule.h"
#include <mutex>

namespace Omni
{
	struct TestAsync
	{
		static constexpr u32 PerThreadLaunch = 1024 * 16;

		struct LauncherJobData
		{
			TestAsync*		State;
			u32				ToLaunch;
		};
		struct DoJobData
		{
			TestAsync*		State;
		};
		TestAsync(u32 workCount)
			: mInFlightCapacity(16)
			, mTodo((u64)workCount * PerThreadLaunch)
		{
		}
		LauncherJobData Prepare(u32)
		{
			LauncherJobData jd;
			jd.State = this;
			jd.ToLaunch = PerThreadLaunch;
			return jd;
		}
		static void DoJob(DoJobData* jd)
		{
			TestAsync& state = *(jd->State);
			{
				std::unique_lock lk(state.mLock);
				--state.mTodo;
				if (state.mTodo == 0)
					state.mCV.notify_all();
			}
			state.mInFlightCapacity.fetch_add(1, std::memory_order_acquire);
		}
		static void Run(LauncherJobData* jd)
		{
			TestAsync& state = *(jd->State);
			while (true)
			{
				if (jd->ToLaunch == 0)
					break;
				--jd->ToLaunch;

				DoJobData djd;
				djd.State = &state;
				DispatchWorkItem& item = DispatchWorkItem::Create(DoJob, &djd, MemoryKind::CacheLine);
				ConcurrencyModule::Get().EnqueueWork(item, QueueKind::Shared);

				i64 ov = state.mInFlightCapacity.fetch_sub(1, std::memory_order_relaxed);
				if (ov <= 0)
				{
					while (state.mInFlightCapacity.load(std::memory_order_relaxed) <= 0)
						PauseThread();
				}
			}
		}
		void Check()
		{
			{
				std::unique_lock lk(mLock);
				while (mTodo > 0)
				{
					mCV.wait(lk);
				}
			}
		}
	private:
		std::mutex				mLock;
		std::condition_variable	mCV;
		std::atomic<i64>		mInFlightCapacity;
		u64						mTodo;
	};
}
