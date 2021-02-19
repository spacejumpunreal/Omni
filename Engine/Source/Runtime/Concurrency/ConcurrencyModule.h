#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Concurrency/ConcurrentDefs.h"
#include "Runtime/Concurrency/JobPrimitives.h"
#include "Runtime/System/Module.h"


namespace Omni
{
	enum class QueueKind : u32;

	class ConcurrencyModule : public Module
	{
	public:
		void Initialize() override;
		void Finalize() override;
		void Finalizing() override;
		static ConcurrencyModule& Get();

		u32 GetWorkerCount();
		DispatchQueue& GetQueue(QueueKind queueKind);
		void Async(DispatchWorkItem& item);
		void DismissWorkers();
	};
}

