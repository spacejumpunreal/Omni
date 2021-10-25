#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/System/System.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Concurrency/ConcurrencyModule.h"
#include "Runtime/Core/Concurrency/ThreadUtils.h"
#include "Runtime/Core/System/Module.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Base/Memory/MonotonicMemoryResource.h"
#include <array>
#include <atomic>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Omni
{
	//consts
	static constexpr size_t SystemInitMemSize = 1024 * 1024;
	static constexpr char LoadModuleText[] = "LoadModule";

#define CoreModuleItem(name) extern ModuleExportInfo MODULE_CREATION_STUB_NAME(name);
#include "Runtime/Core/CoreModuleRegistry.inl"
#undef CoreModuleItem


#define CoreModuleItem(name) &MODULE_CREATION_STUB_NAME(name),
	static const ModuleExportInfo* CoreModuleInfo[] = {
#include "Runtime/Core/CoreModuleRegistry.inl"
	};
#undef CoreModuleItem


	//forward declarations
	struct SystemPrivateData;


	//definitions
	using SystemImpl = PImplCombine<System, SystemPrivateData>;

	struct MainThreadArgs
	{
		u32							Argc;
		const char**				Argv;
		SystemInitializedCallback	OnSystemInitialized;
	};


	struct SystemPrivateData
	{
		std::atomic<SystemStatus>					mStatus;
		std::vector<Module*>						mModules;
		std::unordered_map<ModuleKey, Module*>		mKey2Module;
		std::unordered_map<std::string, Module*>	mName2Module;
		std::vector<ModuleExportInfo>				mExternalModuleInfo;
		MonotonicMemoryResource						mInitMem;
		//ui thread/main thread related
		std::thread									mMainThread;
		std::function<void()>						mUIThreadBody;
		std::mutex									mUIThreadLock;
		std::condition_variable						mUIThreadCV;

		SystemPrivateData()
			: mStatus(SystemStatus::Uninitialized)
			, mInitMem(SystemInitMemSize)
		{}
		void InitializeAndJoinAsMainThread(const MainThreadArgs& args);
		void WaitForUIThreadToStall();
		static void ResumeUIThread(std::function<void()>&& body);
	};


	//globals
	static SystemImpl* GSystem;


	//functions
	void System::CreateSystem()
	{
		CheckAlways(GSystem == nullptr, "double create");
		GSystem = new SystemImpl();
	}

	System& System::GetSystem()
	{
		return *GSystem;
	}

	void System::DestroySystem()
	{
		SystemImpl* self = SystemImpl::GetCombinePtr(this);
		CheckAlways(self->mModules.size() == 0);
		CheckAlways(GSystem != nullptr, "double destroy");
		delete GSystem;
		GSystem = nullptr;
	}

	//this is runned by UI thread
	void System::InitializeAndJoin(u32 argc, const char** argv, SystemInitializedCallback onSystemInitialized)
	{
		SystemImpl* self = SystemImpl::GetCombinePtr(this);
		MainThreadArgs threadArg
		{
			.Argc = argc,
			.Argv = argv,
			.OnSystemInitialized = onSystemInitialized,
		};
		std::unique_lock lk(self->mUIThreadLock);
		self->mMainThread = std::thread([&](){
			RegisterMainThread();
			self->InitializeAndJoinAsMainThread(threadArg);
			UnregisterMainThread();
		});
		self->mUIThreadCV.wait(lk);
		self->mUIThreadBody();
		self->mMainThread.join();
	}

	void System::Finalize()
	{
		SystemImpl* self = SystemImpl::GetCombinePtr(this);
		CheckAlways(self->mStatus == SystemStatus::ToBeFinalized);
		self->mStatus = SystemStatus::Finalizing;
		bool firstRound = true;
		size_t nModules = self->mModules.size();
		size_t todo = 1;
		while (todo > 0)
		{
			todo = 0;
			for (size_t i = 0; i < nModules; ++i)
			{
				Module*& mod = self->mModules[i];
				if (mod != nullptr && (firstRound || mod->GetStatus() == ModuleStatus::Finalizing))
				{
					mod->Finalize();
					ModuleStatus ms = mod->GetStatus();
					switch (ms)
					{
					case ModuleStatus::Finalizing:
						++todo;
						break;
					case ModuleStatus::Uninitialized:
						mod->Destroy();
						mod = nullptr;
						break;
					default:
						CheckAlways(false, "unexpected init status");
						break;
					}
				}
			}
			firstRound = false;
		}
		self->mStatus = SystemStatus::Uninitialized;
		self->mModules.clear();
		self->mKey2Module.clear();
		self->mName2Module.clear();
		self->mExternalModuleInfo.clear();
		self->mInitMem.Reset();
	}

	SystemStatus System::GetStatus()
	{
		const SystemImpl* self = SystemImpl::GetCombinePtr(this);
		return self->mStatus;
	}

	void System::TriggerFinalization(bool assertOnMiss)
	{
		SystemImpl* self = SystemImpl::GetCombinePtr(this);
		SystemStatus v = SystemStatus::Ready;
		if (self->mStatus.compare_exchange_strong(v, SystemStatus::ToBeFinalized))
		{//mark main thread quit, main thread will drive the quit procedure
			DispatchWorkItem& quitJob = DispatchWorkItem::Create(ThreadData::MarkQuitWork, MemoryKind::CacheLine);
			ConcurrencyModule::Get().EnqueueWork(quitJob, QueueKind::Main);
		}
		else if (assertOnMiss)
		{
			CheckAlways(false);
		}
	}

	void System::StopThreads()
	{
		SystemImpl* self = SystemImpl::GetCombinePtr(this);
		for (Module* mod : self->mModules)
		{
			mod->StopThreads();
		}
		ConcurrencyModule::Get().FinishPendingJobs();
	}


	void System::RegisterModule(const ModuleExportInfo& moduleInfo)
	{
		SystemImpl* self = SystemImpl::GetCombinePtr(this);
		self->mExternalModuleInfo.push_back(moduleInfo);
	}

	Module* System::GetModule(ModuleKey key) const
	{
		const SystemImpl* self = SystemImpl::GetCombinePtr(this);
		auto it = self->mKey2Module.find(key);
		if (it == self->mKey2Module.end())
			return nullptr;
		return it->second;
	}

	Module* System::GetModule(const char* s) const
	{
		const SystemImpl* self = SystemImpl::GetCombinePtr(this);
		auto it = self->mName2Module.find(s);
		if (it == self->mName2Module.end())
			return nullptr;
		return it->second;
	}

	MonotonicMemoryResource& System::GetInitMemResource()
	{
		SystemImpl* self = SystemImpl::GetCombinePtr(this);
		return self->mInitMem;
	}

	void SystemPrivateData::InitializeAndJoinAsMainThread(const MainThreadArgs& args)
	{
		SystemImpl* self = SystemImpl::GetCombinePtr(this);
		//parse args
		EngineInitArgMap argMap;
		for (u32 i = 0; i < args.Argc; ++i)
		{
			size_t b = 0;
			const char* arg = args.Argv[i];
			while (arg[b] != 0 && arg[b] != '=')
				++b;
			std::string k(arg, arg + b);
			std::string v;
			if (arg[b] == '=')
			{
				++b;
				size_t bb = b;
				while (arg[b] != 0)
					++b;
				v = std::string(arg + bb, arg + b);
			}
			argMap.insert(std::make_pair(k, v));
		}
		//loadModuleNames
		auto tup = argMap.equal_range(LoadModuleText);
		std::unordered_set<std::string> loadModuleNames;
		for (auto it = tup.first; it != tup.second; ++it)
			loadModuleNames.insert(it->second);

		self->mStatus = SystemStatus::Initializing;
		//create internal modules
		for (size_t i = 0; i < ARRAY_LENGTH(CoreModuleInfo); ++i)
		{
			auto info = CoreModuleInfo[i];
			if (info->IsAlwaysLoad ||
				(CoreModuleInfo[i]->Name != nullptr && loadModuleNames.count(CoreModuleInfo[i]->Name) != 0))
			{
				Module* m = info->Ctor(argMap);
				CheckAlways(info->Key == -1, "intenral modules should all have -1 keys in declaration, actually keys will come from order in list");
				self->mModules.push_back(m);
				self->mKey2Module[(ModuleKey)i] = m;
				if (info->Name != nullptr)
					self->mName2Module[info->Name] = m;
			}
		}
		//create external modules
		for (size_t i = 0; i < self->mExternalModuleInfo.size(); ++i)
		{
			const ModuleExportInfo& info = self->mExternalModuleInfo[i];
			if (info.IsAlwaysLoad || loadModuleNames.count(info.Name) != 0)
			{
				Module* m = info.Ctor(argMap);
				self->mModules.push_back(m);
				if (info.Key >= 0)
				{
					CheckAlways(self->mKey2Module.count(info.Key) == 0, "dumplicated module keys");
					self->mKey2Module[info.Key] = m;
				}
				if (info.Name != nullptr)
				{
					CheckAlways(self->mName2Module.count(info.Name) == 0, "duplicated module names");
					self->mName2Module[info.Name] = m;
				}
			}
		}
		size_t nModules = self->mModules.size();
		size_t todo = 1;//make sure we enter the first round
		bool firstRound = true;
		while (todo > 0)
		{
			todo = 0;
			for (size_t i = 0; i < nModules; ++i)
			{
				Module* mod = self->mModules[i];
				if (firstRound || mod->GetStatus() == ModuleStatus::Initializing)
				{
					mod->Initialize(argMap);
				}
				ModuleStatus ms = mod->GetStatus();
				switch (ms)
				{
				case ModuleStatus::Initializing:
					++todo;
					break;
				case ModuleStatus::Ready:
					break;
				default:
					CheckAlways(false, "unexpected init status");
					break;
				}
			}
			firstRound = false;
		}
		//free it anyway, if already previously freed, this will have no effect
		self->ResumeUIThread(std::function<void()>([]() {}));
		self->mStatus = SystemStatus::Ready;
		ThreadData::GetThisThreadData().RunAndFinalizeAsMain(args.OnSystemInitialized);
	}

	void SystemPrivateData::WaitForUIThreadToStall()
	{
		SystemImpl* self = SystemImpl::GetCombinePtr(this);
		self->mUIThreadLock.lock();
		self->mUIThreadLock.unlock();
	}
	void SystemPrivateData::ResumeUIThread(std::function<void()>&& body)
	{
		SystemImpl* self = SystemImpl::GetCombinePtr(&System::GetSystem());
		self->mUIThreadLock.lock();
		self->mUIThreadBody = std::move(body);
		self->mUIThreadLock.unlock();
		self->mUIThreadCV.notify_one();
	}
}
