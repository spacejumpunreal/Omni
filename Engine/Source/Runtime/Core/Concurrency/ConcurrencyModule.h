#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/Concurrency/ConcurrentDefs.h"
#include "Runtime/Core/Concurrency/JobPrimitives.h"
#include "Runtime/Core/System/Module.h"


namespace Omni
{
	enum class QueueKind : u32;

	class CORE_API ConcurrencyModule : public Module
	{
	public:
		void Destroy() override;
		void Initialize(const EngineInitArgMap&) override;
		void Finalize() override;
		static ConcurrencyModule& Get();

		u32 GetWorkerCount();
		DispatchQueue& GetQueue(QueueKind queueKind);
		void Async(DispatchWorkItem& item);
		void DismissWorkers();
	};
}

