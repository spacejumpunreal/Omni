#include "Runtime/System/System.h"
#include "Runtime/Concurrency/ConcurrencyModule.h"
#include "Runtime/Concurrency/JobPrimitives.h"
#include "Runtime/Concurrency/SpinLock.h"
#include "Runtime/Concurrency/ThreadUtils.h"
#include "Runtime/Memory/MemoryArena.h"
#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Misc/ArrayUtils.h"
#include "Runtime/Platform/PlatformAPIs.h"
#include "Runtime/Test/AssertUtils.h"
#include "Runtime/Test/PerfUtils.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <random>
#include <thread>
#include <unordered_set>


namespace Omni
{
	struct AllocJobData
	{
		size_t				IThread;
		DispatchGroup*		Group;
	};

	void AllocJobFunc0(AllocJobData* jobData)
	{
        
		constexpr size_t Size64K = 64 * 1024;
		constexpr size_t Amount = 1024 * 1024 * 256;
		constexpr size_t History = 8;

		PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::CacheLine);
		std::mt19937 gen;
		gen.seed((unsigned int)jobData->IThread);
		std::uniform_int_distribution<> dis((int)(Size64K / 3), (int)(Size64K * 3 / 4));
		u8* slots[History];
		size_t sizes[History];
		memset(slots, 0, sizeof(slots));
		memset(sizes, 0, sizeof(sizes));
		size_t allocated = 0;
		size_t slotIndex = 0;
		int runIndex = 0;
		while (allocated < Amount)
		{
			u8* u8Ptr = slots[slotIndex];
			size_t sz = sizes[slotIndex];
			u8 w = (u8)sz;
			if (sz != 0)
			{
				bool bad = false;
				for (size_t iBytes = 0; iBytes < sz; ++iBytes)
				{
					if (u8Ptr[iBytes] != w)
					{
						bad = true;
						break;
					}
				}
				CheckAlways(!bad);
				alloc.resource()->deallocate(slots[slotIndex], 0);
			}
			size_t size = (size_t)dis(gen);
			std::byte* p = alloc.allocate(size);
			memset(p, (int)size, size);
			slots[slotIndex] = (u8*)p;
			sizes[slotIndex] = size;
			slotIndex = (slotIndex + 1) % History;
			allocated += size;
			++runIndex;
		}
		for (size_t iSlot = 0; iSlot < History; ++iSlot)
		{
			if (slots[iSlot])
			{
				alloc.resource()->deallocate(slots[iSlot], 0);
			}
		}
		jobData->Group->Leave();
	}
	struct MainQueueJobState
	{
		SpinLock	TestLock;
		u64			RunOnHistory[16];
		u64			Sequence;
		u64			ExpectedSequence;
		u64			NoUse;
		MainQueueJobState()
			: Sequence(0)
			, ExpectedSequence(0)
			, NoUse(0)
		{
			memset(RunOnHistory, 0, sizeof(RunOnHistory));
		}
	};
	struct MainQueueJobData
	{
		MainQueueJobState*	State;
		u64					Sequence;
	};
	void MainQueueJobOnly(MainQueueJobData* p)
	{
		MainQueueJobState* state = p->State;
		CheckAlways(p->Sequence == state->Sequence);

		++state->Sequence;
		CheckAlways(state->TestLock.TryLock());
		u32 tid = (u32)ThreadData::GetThisThreadData().GetThreadIndex();
		++state->RunOnHistory[tid];
		state->NoUse = TimeConsumingFunctions::Fab(20);
		state->TestLock.Unlock();
	}
	void MainQueueQuitJob(MainQueueJobState** state)
	{
		CheckAlways((*state)->Sequence == (*state)->ExpectedSequence);
		OMNI_DELETE(*state, MemoryKind::UserDefault);
		System::GetSystem().TriggerFinalization();
	}
	void AllocJobFuncDone()
	{
		constexpr size_t NSeq = 1024;
#if false
		System::GetSystem().TriggerFinalization();
#else
		MainQueueJobState* state = OMNI_NEW(MemoryKind::UserDefault) MainQueueJobState();
		state->ExpectedSequence = NSeq;
		MainQueueJobData jd;

		jd.Sequence = 0;
		jd.State = state;
		
		DispatchWorkItem* firstJob = nullptr, *lastJob = nullptr;
		for (size_t iJob = 0; iJob < NSeq; ++iJob)
		{
			DispatchWorkItem& p = DispatchWorkItem::Create(MainQueueJobOnly, &jd);
			if (!firstJob)
				firstJob = &p;
			if (lastJob)
				lastJob->Next = &p;
			lastJob = &p;
			++jd.Sequence;
		}
		DispatchWorkItem& finalJob = DispatchWorkItem::Create(MainQueueQuitJob, &state);
		lastJob->Next = &finalJob;
		ConcurrencyModule::Get().GetQueue(QueueKind::Primary).Enqueue(firstJob, &finalJob);
#endif
	}

	struct JobParallelAddToQueueData
	{
	public:
		static constexpr int	PerThreadLaunch = 1024 * 16;
		std::atomic<int>		mInFlightCapacity;
		std::atomic<int>		mTodo;
		DispatchGroup*			mGroup;
	public:
		JobParallelAddToQueueData()
			: mInFlightCapacity(16)
			, mTodo(0)
			, mGroup(0)
		{
		}
		struct LauncherJobData
		{
			JobParallelAddToQueueData*	State;
			u32							ToLaunch;
		};
		struct DoJobData
		{
			JobParallelAddToQueueData* State;
		};
		static void LauncherJob(LauncherJobData* jd)
		{
			JobParallelAddToQueueData& state = *(jd->State);
			while (true)
			{
				if (jd->ToLaunch == 0)
					break;
				--jd->ToLaunch;

				DoJobData djd;
				djd.State = &state;
				DispatchWorkItem& item = DispatchWorkItem::Create(DoJob, &djd);
				ConcurrencyModule::Get().Async(item);

				int ov = state.mInFlightCapacity.fetch_sub(1, std::memory_order_relaxed);
				if (ov <= 0)
				{
					while (state.mInFlightCapacity.load(std::memory_order_relaxed) <= 0)
						PauseThread();
				}
			}
			state.mGroup->Leave();
		}
		static void DoJob(DoJobData* jd)
		{
			jd->State->mInFlightCapacity.fetch_add(1, std::memory_order_acquire);
			int nv = jd->State->mTodo.fetch_sub(1, std::memory_order_relaxed);
			if (nv == 1)
			{
				jd->State->mGroup->Leave();
				System::GetSystem().TriggerFinalization();
			}
		}
		static void CleanupJob(JobParallelAddToQueueData** pp)
		{
			OMNI_DELETE(*pp, MemoryKind::UserDefault);
		}
	};

	void MainThreadTest()
	{
#if true
		{
			PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(Omni::MemoryKind::UserDefault);
			size_t testSize = 1024;
			std::byte* p = alloc.allocate(testSize);
			memset(p, 0, testSize);
			alloc.deallocate(p, testSize);
		}
		{
			ScratchStack& stack = MemoryModule::Get().GetThreadScratchStack();
			stack.Push();
			{
				MemoryArenaScope s0 = stack.PushScope();
				void* f = nullptr;
				{
					MemoryArenaScope s1 = stack.PushScope();
					f = stack.Allocate(3);
					constexpr u32 allocSizes[] = { 1, 2, 4, 8, 12, 17, 19, 354 };
					for (u32 sz : allocSizes)
					{
						stack.Allocate(sz);
					}
					void* m = nullptr;
					{
						MemoryArenaScope s2 = stack.PushScope();
						m = stack.Allocate(1);
						for (u32 sz : allocSizes)
						{
							stack.Allocate(sz);
						}
					}
					void* m1 = stack.Allocate(2);
					CheckAlways(m1 == m);
				}
				void* f1 = stack.Allocate(1);
				CheckAlways(f1 == f);
			}
			stack.Pop();
		}
		{
			SpinLock sl;
			
			sl.Lock();
			std::thread x = std::thread([&]
			{
				//std::this_thread::sleep_for(std::chrono::seconds(5));
				sl.Unlock();
			});
			sl.Lock();
			bool succeeed = sl.TryLock();
			CheckAlways(!succeeed);
			sl.Unlock();
			x.join();
		}
#endif
#if true
		{//test job threads allocation and then serial queue execution
			constexpr size_t NThreads = 8;
			DispatchGroup& group = DispatchGroup::Create(0);
			AllocJobData jd;
			jd.Group = &group;
			DispatchWorkItem* lastJob = nullptr;
			for (size_t iThread = 0; iThread < NThreads; ++iThread)
			{
				jd.IThread = iThread;
				DispatchWorkItem& item = DispatchWorkItem::Create(AllocJobFunc0, &jd);
				item.Next = lastJob;
				lastJob = &item;
				group.Enter();
			}
			DispatchWorkItem& item = DispatchWorkItem::Create(AllocJobFuncDone);
			group.Notify(item, nullptr);
			ConcurrencyModule::Get().Async(*lastJob);
		}
#endif
		{
			size_t NJobs = ConcurrencyModule::Get().GetWorkerCount() - 2;
			JobParallelAddToQueueData* state = OMNI_NEW(MemoryKind::UserDefault)JobParallelAddToQueueData();
			
			JobParallelAddToQueueData::LauncherJobData jd;
			jd.State = state;
			DispatchWorkItem* p = nullptr;
			int todo = 0;
			for (size_t iJob = 0; iJob < NJobs; ++iJob)
			{
				jd.ToLaunch = (u32)JobParallelAddToQueueData::PerThreadLaunch;
				todo += jd.ToLaunch;
				DispatchWorkItem& item = DispatchWorkItem::Create(JobParallelAddToQueueData::LauncherJob, &jd);
				item.Next = p;
				p = &item;
			}
			state->mTodo = todo;
			state->mGroup = &DispatchGroup::Create((u32)(NJobs + 1));
			DispatchWorkItem& cleanupJob = DispatchWorkItem::Create(JobParallelAddToQueueData::CleanupJob, &state);
			state->mGroup->Notify(cleanupJob, nullptr);
			ConcurrencyModule::Get().Async(*p);
		}
	}
}

int main(int, const char** )
{
	Omni::System::CreateSystem();
	Omni::System& system = Omni::System::GetSystem();
	const char* engineArgv[] =
	{
		"",
	};
	system.InitializeAndJoin(ARRAY_LENGTH(engineArgv), engineArgv, Omni::MainThreadTest);
	system.DestroySystem();
 
	return 0;
}
