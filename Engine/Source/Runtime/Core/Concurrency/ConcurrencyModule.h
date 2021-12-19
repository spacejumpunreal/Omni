#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Misc/TimeTypes.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/Concurrency/ConcurrentDefs.h"
#include "Runtime/Core/Concurrency/ThreadUtils.h"
#include "Runtime/Core/Concurrency/JobPrimitives.h"
#include "Runtime/Core/System/Module.h"


namespace Omni
{
	//forward decls
	class LockQueue;
	class ThreadData;
	enum class QueueKind : u32;


	class CORE_API ConcurrencyModule : public Module
	{
	public:
		void Destroy() override;
		void Initialize(const EngineInitArgMap&) override;
		void StopThreads() override;
		void Finalize() override;
		static ConcurrencyModule& Get();

		u32 GetWorkerCount();
		LockQueue* GetQueue(QueueKind queueKind);
		ThreadData* CreateThread(const TThreadBody& body, ThreadId designatedTid = InvalidThreadId);
		//for already existing thread, called on MainThread, return value should be passed to the existing thread, won't join it, but will check if it's finalized
		ThreadData* RegisterExternalThread(ThreadId designatedTid = InvalidThreadId, const wchar_t* extThreadName = L"OmniExternalThread");
		void EnqueueWork(DispatchWorkItem& head, QueueKind queueKind);
		void PollQueueUntil(QueueKind queueKind, const TimePoint* deadline);
		void PollQueue(QueueKind queueKind);
		void FinishPendingJobs();
	};
}

