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

	template<typename TestLogic>
	class RunOnEveryWorker
	{
	private:
		using TRawJobData = decltype(TestLogic{0}.Prepare(0));
		struct WrapperJobData
		{
			TRawJobData			RawData;
			DispatchGroup*		Group;
			RunOnEveryWorker*	ThisData;
		};
	public:
		void OnWorkerDone()
		{
			{
				std::unique_lock ul(mLock);
				--mTodo;
				if (mTodo == 0)
					mCVWorker.notify_all();
			}
			{
				std::unique_lock ul(mLock);
				while (mTodo > 0)
					mCVWorker.wait(ul);
			}
			{
				std::unique_lock ul(mLock);
				--mToWait;
				if (mToWait == 0)
					mCVMain.notify_all();
			}
		}
		RunOnEveryWorker(u32 idleWorkers = 0)
			: mTodo(ConcurrencyModule::Get().GetWorkerCount() - 1 - idleWorkers)
			, mToWait(ConcurrencyModule::Get().GetWorkerCount() - 1 - idleWorkers)
			, mLogic(ConcurrencyModule::Get().GetWorkerCount() - 1 - idleWorkers)
		{
			CheckAlways(IsOnMainThread());
			DispatchGroup& group = DispatchGroup::Create(mTodo);
			DispatchWorkItem* head = nullptr;
			for (u32 iWork = 0; iWork < mTodo; ++iWork)
			{
				WrapperJobData jobData;
				jobData.RawData = mLogic.Prepare(iWork);
				jobData.Group = &group;
				jobData.ThisData = this;
				DispatchWorkItem* item = &DispatchWorkItem::Create(&WrapperBody, &jobData);
				item->Next = head;
				head = item;
			}
			ConcurrencyModule::Get().Async(*head);
			WaitDone();
			mLogic.Check();
		}
		void WaitDone()
		{
			std::unique_lock ul(mLock);
			while (mToWait > 0)
			{
				mCVMain.wait(ul);
			}
		}
	private:
		std::mutex					mLock;
		std::condition_variable		mCVWorker;
		std::condition_variable		mCVMain;
		u32							mTodo;
		u32							mToWait;
		TestLogic					mLogic;
	private:
		static void WrapperBody(WrapperJobData* data)
		{
			TestLogic::Run(&data->RawData);
			data->Group->Leave();
			data->ThisData->OnWorkerDone();
		}
	};
}
