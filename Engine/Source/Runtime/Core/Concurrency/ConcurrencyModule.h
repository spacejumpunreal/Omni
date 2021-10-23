#pragma once
#include "Runtime/Prelude/Omni.h"
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
		void EnqueueWork(DispatchWorkItem& head, QueueKind queueKind);
		void SignalWorkersToQuit();
	};
}

