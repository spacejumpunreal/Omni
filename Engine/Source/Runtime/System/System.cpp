#include "Runtime/System/System.h"
#include "Runtime/Concurrency/ConcurrencyModule.h"
#include "Runtime/Concurrency/ThreadUtils.h"
#include "Runtime/System/Module.h"
#include "Runtime/System/ModuleExport.h"
#include "Runtime/System/InternalModuleRegistry.h"
#include "Runtime/Misc/PImplUtils.h"
#include "Runtime/Misc/ArrayUtils.h"
#include "Runtime/Test/AssertUtils.h"
#include "Runtime/Memory/MemoryModule.h"



#include <array>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <atomic>

namespace Omni
{
	//forward declarations
	struct SystemPrivateData;

	//definitions
	using SystemImpl = PImplCombine<System, SystemPrivateData>;
	struct SystemPrivateData
	{
		std::atomic<SystemStatus>					mStatus;
		std::vector<Module*>						mModules;
		std::unordered_map<ModuleKey, Module*>		mKey2Module;
		std::unordered_map<std::string, Module*>	mName2Module;
		std::vector<ModuleExportInfo>				mExternalModuleInfo;

		SystemPrivateData()
			: mStatus(SystemStatus::Uninitialized)
		{}
	};

	//constants
#define ModuleItem(name) extern ModuleExportInfo ModuleCreationStubName(name);
#include "Runtime/System/InternalModuleRegistry.inl"
#undef ModuleItem

#define ModuleItem(name) &ModuleCreationStubName(name),
	static const ModuleExportInfo* InternalModuleInfo[] = {
#include "Runtime/System/InternalModuleRegistry.inl"
	};
#undef ModuleItem


	static const char* LoadModuleText = "LoadModule";

	//globals
	static SystemImpl* GSystem;

	//functions
	void System::CreateSystem()
	{
		RegisterMainThread();
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
		UnregisterMainThread();
	}
	void System::InitializeAndJoin(u32 argc, const char** argv, SystemInitializedCallback onSystemInitialized)
	{
		//parse args
		EngineInitArgMap argMap;
		for (u32 i = 0; i < argc; ++i)
		{
			usize b = 0;
			const char* arg = argv[i];
			while (arg[b] != 0 && arg[b] != '=')
				++b;
			std::string k(arg, arg + b);
			std::string v;
			if (arg[b] == '=')
			{
				++b;
				usize bb = b;
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
		SystemImpl* self = SystemImpl::GetCombinePtr(this);
		self->mStatus = SystemStatus::Initializing;
		//create internal modules
		for (size_t i = 0; i < ARRAY_LENGTH(InternalModuleInfo); ++i)
		{
			auto info = InternalModuleInfo[i];
			if (info->IsAlwaysLoad || (InternalModuleInfo[i]->Name != nullptr && loadModuleNames.count(InternalModuleInfo[i]->Name) != 0))
			{
				Module* m = info->Ctor(argMap);
				self->mModules.push_back(m);
				self->mKey2Module[(ModuleKey)i] = m;
				if (info->Name != nullptr)
					self->mName2Module[info->Name] = m;
			}
		}
		//create external modules
		for (usize i = 0; i < self->mExternalModuleInfo.size(); ++i)
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
		usize nModules = self->mModules.size();
		usize todo = 1;//make sure we enter the first round
		bool firstRound = true;
		while (todo > 0)
		{
			todo = 0;
			for (usize i = 0; i < nModules; ++i)
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
		self->mStatus = SystemStatus::Ready;
		ThreadData::GetThisThreadData().RunAndFinalizeOnMain(onSystemInitialized);

	}
	void System::Finalize()
	{
		SystemImpl* self = SystemImpl::GetCombinePtr(this);
		CheckAlways(self->mStatus == SystemStatus::ToBeFinalized);
		self->mStatus = SystemStatus::Finalizing;
		bool firstRound = true;
		usize nModules = self->mModules.size();
		usize todo = 1;
		while (todo > 0)
		{
			todo = 0;
			for (usize i = 0; i < nModules; ++i)
			{
				Module* mod = self->mModules[i];
				if (firstRound || mod->GetStatus() == ModuleStatus::Finalizing)
				{
					mod->Finalize();
				}
				ModuleStatus ms = mod->GetStatus();
				switch (ms)
				{
				case ModuleStatus::Finalizing:
					++todo;
					break;
				case ModuleStatus::Uninitialized:
					break;
				default:
					CheckAlways(false, "unexpected init status");
					break;
				}
			}
			firstRound = false;
		}
		for (Module* mod : self->mModules)
		{
			delete mod;
		}
		self->mStatus = SystemStatus::Uninitialized;
		self->mModules.clear();
		self->mKey2Module.clear();
		self->mName2Module.clear();
		self->mExternalModuleInfo.clear();
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
			ConcurrencyModule::Get().DismissWorkers();
		else if (assertOnMiss)
		{
			CheckAlways(false);
		}
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
}
