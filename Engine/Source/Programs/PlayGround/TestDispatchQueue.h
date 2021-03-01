#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Concurrency/ConcurrencyModule.h"
#include "Runtime/Concurrency/JobPrimitives.h"
#include "Runtime/Concurrency/SpinLock.h"
#include "Runtime/Concurrency/ThreadUtils.h"
#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Platform/PlatformAPIs.h"
#include "Runtime/Test/AssertUtils.h"
#include "Runtime/Test/PerfUtils.h"
#include <random>
#include <chrono>

namespace Omni
{
	struct TestDispatchQueue
	{
		static constexpr u64 JobBatch = 256;
		static constexpr u64 JobLoop = 256;
		static constexpr u32 MaxThreads = 32;
		struct JobData
		{
			TestDispatchQueue*	State;
			u64					Sequence;
		};

		TestDispatchQueue(u32 workerCount)
			: TodoCount((u64)workerCount * JobBatch * JobLoop)
			, ExpectedSum(0)
			, Sum(0)
		{
			memset(RunOnHistory, 0, sizeof(RunOnHistory));
			u64 n = workerCount * (JobBatch * JobLoop);
			ExpectedSum = n * (n - 1) / 2;
		}

		JobData Prepare(u32 i)
		{
			JobData jd;
			jd.State = this;
			jd.Sequence = i * JobBatch * JobLoop;
			return jd;
		}
		struct SerialJobData
		{
			TestDispatchQueue*		State;
			u64						Sequence;
		};
		static void SerialJob(SerialJobData* jd)
		{
			TestDispatchQueue* state = jd->State;
			CheckAlways(state->TestLock.TryLock());
			u32 tid = (u32)ThreadData::GetThisThreadData().GetThreadIndex();
			++state->RunOnHistory[tid];
			TimeConsumingFunctions::Fab(5);
			jd->State->Sum += jd->Sequence;
			state->TestLock.Unlock();
			jd->State->TodoCount.fetch_sub(1, std::memory_order_release);
		}
		static void Run(JobData* jd)
		{
			SerialJobData tmpJd;
			tmpJd.State = jd->State;
			tmpJd.Sequence = jd->Sequence;
			for (u32 i = 0; i < JobLoop; ++i)
			{
				DispatchWorkItem* head = nullptr;
				DispatchWorkItem* tail = nullptr;
				for (u32 j = 0; j < JobBatch; ++j)
				{
					DispatchWorkItem& item = DispatchWorkItem::Create(&SerialJob, &tmpJd);
					if (j == 0)
						tail = &item;
					++tmpJd.Sequence;
					item.Next = head;
					head = &item;
				}
				ConcurrencyModule::Get().GetQueue(QueueKind::Primary).Enqueue(head, tail);
				if (i % 10 == 0)
				{
					using namespace std::chrono_literals;
					std::this_thread::sleep_for(1ms);
				}
			}
		}
		void Check()
		{
			while (TodoCount.load(std::memory_order_acquire) != 0)
				PauseThread();
			CheckAlways(Sum == ExpectedSum);
		}
	public:
		SpinLock				TestLock;
		u64						RunOnHistory[MaxThreads];
		std::atomic<u64>		TodoCount;
		u64						ExpectedSum;
		u64						Sum;
	};
}