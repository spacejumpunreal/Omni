#pragma once
#include "Omni.h"
#include "Concurrency/ConcurrentDefs.h"
#include "Concurrency/JobPrimitives.h"
#include "System/Module.h"


namespace Omni
{
	enum class QueueKind : u32;

	class CORE_API ConcurrencyModule : public Module
	{
	public:
		void Destroy() override;
		void Initialize(const EngineInitArgMap&) override;
		void Finalize() override;
		void Finalizing() override;
		static ConcurrencyModule& Get();

		u32 GetWorkerCount();
		DispatchQueue& GetQueue(QueueKind queueKind);
		void Async(DispatchWorkItem& item);
		void DismissWorkers();
	};
}

