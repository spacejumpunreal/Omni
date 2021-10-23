#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/Concurrency/ConcurrencyModule.h"
#include "Runtime/Core/Concurrency/JobPrimitives.h"
#include "Runtime/Core/Concurrency/ThreadUtils.h"
#include <chrono>
#include <mutex>

namespace Omni
{
	// use this to run a job on "ConcurrencyModule::Get().GetWorkerCount() - idleWorkers - 1" workers,
	// guaranteed that no worker will execute this job twice and that every job thread(including calling thread) will execute this job
	//notice that current thread is also counted
	template<typename JobLogic>
	class RunOnEveryWorker
	{
	private:
		using TRawJobData = decltype(JobLogic{0}.Prepare(0));
		struct WrapperJobData
		{
			TRawJobData			RawData;
			RunOnEveryWorker*	ThisData;
		};
	public:
		RunOnEveryWorker(u32 idleWorkers = 0)
			: mTodo(ConcurrencyModule::Get().GetWorkerCount() - idleWorkers)
			, mRefCount(ConcurrencyModule::Get().GetWorkerCount() - idleWorkers - 1)
			, mLogic(ConcurrencyModule::Get().GetWorkerCount() - idleWorkers)
		{
			CheckAlways(IsOnMainThread());
			DispatchWorkItem* head = nullptr;
			WrapperJobData jobData;
			for (u32 iWork = 0; iWork < mTodo; ++iWork)
			{
				jobData.RawData = mLogic.Prepare(iWork);
				jobData.ThisData = this;
				if (iWork != mTodo - 1) //leave the last for local execution
				{
					DispatchWorkItem* item = &DispatchWorkItem::Create(&WrapperBody, &jobData, MemoryKind::CacheLine);
					item->Next = head;
					head = item;
				}
			}
			ConcurrencyModule::Get().EnqueueWork(*head, QueueKind::Shared);
			WrapperBody(&jobData);
			mLogic.Check();
		}
	private:
		std::mutex					mLock;
		u32							mTodo;
		u32							mRefCount;
		JobLogic					mLogic;
	private:
		static void WrapperBody(WrapperJobData* data)
		{
			JobLogic::Run(&data->RawData);
			{
				std::unique_lock lk(data->ThisData->mLock);
				--data->ThisData->mTodo;
			}
			while (true)
			{
				std::unique_lock lk(data->ThisData->mLock);
				if (data->ThisData->mTodo == 0)
					break;
			}
			//the following ensures that MainThread is the last thread
			if (IsOnMainThread())
			{
				while (true)
				{
					std::unique_lock lk(data->ThisData->mLock);
					if (data->ThisData->mRefCount == 0)
						break;
				}
			}
			else
			{
				std::unique_lock lk(data->ThisData->mLock);
				--data->ThisData->mRefCount;
			}
		}
	};
}
