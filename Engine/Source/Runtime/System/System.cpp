#include "Runtime/Omni.h"
#include "Runtime/System/System.h"
#include "Runtime/System/Module.h"
#include "Runtime/System/ModuleExport.h"
#include "Runtime/System/InternalModuleRegistry.h"
#include "Runtime/Misc/PImplUtils.h"
#include "Runtime/Misc/ArrayUtils.h"
#include "Runtime/Misc/AssertUtils.h"


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
	void System::CreateSystem()
	{
		GSystem = new SystemImpl();
	}
	System& System::GetSystem()
	{
		return *GSystem;
	}
	void System::DestroySystem()
	{
		delete GSystem;
		GSystem = nullptr;
	}
	void System::InitializeAndJoin(u32 argc, const char** argv)
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
		auto tup = argMap.equal_range(LoadModuleText);
		std::unordered_set<std::string> loadModuleNames;
		for (auto it = tup.first; it != tup.second; ++it)
			loadModuleNames.insert(it->second);
		SystemImpl* self = SystemImpl::GetCombinePtr(this);
		self->mStatus = SystemStatus::Initializing;
		//create internal modules
		for (ModuleKey i = 0; i < ARRAY_LENGTH(InternalModuleInfo); ++i)
		{
			auto info = InternalModuleInfo[i];
			if (info->IsAlwaysLoad || loadModuleNames.count(InternalModuleInfo[i]->Name) != 0)
			{
				Module* m = info->Ctor(argMap);
				self->mModules.push_back(m);
				self->mKey2Module[i] = m;
				if (info->Name != nullptr)
					self->mName2Module[info->Name] = m;
			}
		}
		for (usize i = 0; i < self->mExternalModuleInfo.size(); ++i)
		{
			auto info = self->mExternalModuleInfo[i];
			if (info.IsAlwaysLoad || loadModuleNames.count(InternalModuleInfo[i]->Name) != 0)
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
		usize todo = 0;
		for (usize i = 0; i < nModules; ++i)
		{
			Module* mod = self->mModules[i];
			mod->Initialize();
			if (mod->GetState() == ModuleStatus::Initializing)
				++todo;
		}
		while (todo > 0)
		{
			todo = 0;
			for (usize i = 0; i < nModules; ++i)
			{
				Module* mod = self->mModules[i];
				if (mod->GetState() == ModuleStatus::Initializing)
				{
					mod->Initializing();
					if (mod->GetState() == ModuleStatus::Initializing)
						++todo;
				}
			}
		}
		self->mStatus = SystemStatus::Ready;
		//join concurrency module
	}

	SystemStatus System::GetState()
	{
		const SystemImpl* self = SystemImpl::GetCombinePtr(this);
		return self->mStatus;
	}

	void System::TriggerFinalization()
	{
		SystemImpl* self = SystemImpl::GetCombinePtr(this);
		if (self->mStatus == SystemStatus::Ready)
			self->mStatus = SystemStatus::ToBeFinalized;
	}

	void System::WaitTillFinalized()
	{
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
