#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/Concurrency/ConcurrencyModule.h"
#include "Runtime/Core/Concurrency/JobPrimitives.h"
#include "Runtime/Core/Concurrency/ThreadUtils.h"
#include <chrono>
#include <mutex>
#include <thread>

namespace Omni
{
	template<typename T>
	class HasIdleWaitMethod
	{//standard use of SFINAE: https://en.wikipedia.org/wiki/Substitution_failure_is_not_an_error
		template<typename C> static std::true_type Test(decltype(&C::IdleWait));
		template<typename C> static std::false_type Test(...);
	public:
		using Type = decltype(Test<T>(nullptr));
		static constexpr bool Value = Type::value;
	};

	// use this to run a job on "ConcurrencyModule::Get().GetWorkerCount() - idleWorkers" workers
	// guaranteed that no worker will execute this job twice and that every job thread will execute this job
	// main thread(the calling thread will just busy wait
	template<typename JobLogic>
	class RunOnAllWorker
	{
	private:
		using TRawJobData = decltype(JobLogic{0}.Prepare(0));
		struct WrapperJobData
		{
			TRawJobData			RawData;
			RunOnAllWorker*		ThisData;
		};
	public:
		RunOnAllWorker(u32 idleWorkers = 0)
			: mTodo(ConcurrencyModule::Get().GetWorkerCount() - idleWorkers)
			, mLogic(ConcurrencyModule::Get().GetWorkerCount() - idleWorkers)
		{
			CheckAlways(IsOnMainThread());
			DispatchWorkItem* head = nullptr;
			WrapperJobData jobData;
			for (u32 iWork = 0; iWork < mTodo; ++iWork)
			{
				jobData.RawData = mLogic.Prepare(iWork);
				jobData.ThisData = this;
				DispatchWorkItem* item = &DispatchWorkItem::Create(&WorkerFunc, &jobData, MemoryKind::CacheLine, true);
				item->Next = head;
				head = item;
			}
			ConcurrencyModule::Get().EnqueueWork(*head, QueueKind::Shared);
			while (true)
			{
				std::unique_lock lk(mLock);
				if (mTodo == 0)
					break;
				if constexpr (HasIdleWaitMethod<JobLogic>::Value)
				{
					mLogic.IdleWait();
				}
				else
				{
					std::this_thread::yield();
				}
				
			}
			mLogic.Check();
		}
	private:
		std::mutex					mLock;
		u32							mTodo;
		JobLogic					mLogic;
	private:
		static void WorkerFunc(WrapperJobData* data)
		{
			JobLogic::Run(&data->RawData);
			{
				std::unique_lock lk(data->ThisData->mLock);
				--data->ThisData->mTodo;
			}
		}
	};
}
