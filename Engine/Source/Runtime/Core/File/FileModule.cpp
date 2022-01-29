#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/File/FileModule.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/Text/TextEncoding.h"
#include "Runtime/Core/System/Module.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"
#include <filesystem>
#include <fstream>

namespace Omni
{

/*
 * Definitions
 */
struct FileModulePrivateData
{
public:
    PMRUnorderedMap<RootPathKey, Path> RegisteredRoots;

public:
    FileModulePrivateData();
};

using FileModuleImpl = PImplCombine<FileModule, FileModulePrivateData>;

/*
 * globals
 */
static FileModuleImpl* gFileModule;

/*
 * Method definitions
 */

// FileModulePrivateData
FileModulePrivateData::FileModulePrivateData() : RegisteredRoots(MemoryModule::Get().GetPMRAllocator(MemoryKind::SystemInit))
{
    CheckAlways(gFileModule == nullptr, "singleton rule violated");
    gFileModule = FileModuleImpl::GetCombinePtr(this);
}

// FileModule
Module* FileModuleCtor(const EngineInitArgMap&)
{
    return InitMemFactory<FileModuleImpl>::New();
}

void FileModule::Destroy()
{
    return InitMemFactory<FileModuleImpl>::Delete((FileModuleImpl*)this);
}

void FileModule::Initialize(const EngineInitArgMap& args)
{
    MemoryModule::Get().Retain();
    Module::Initialize(args);
    auto itr = args.find("--file-project-root");
    if (itr != args.end())
    {
        std::filesystem::path tpath = std::filesystem::path(itr->second.c_str());
        RegisterRoot(PredefinedPath::ProjectRoot, tpath.c_str());
    }
}

void FileModule::Finalize()
{
    //FileModulePrivateData* self = FileModuleImpl::GetCombinePtr(this);
    Module::Finalizing();
    if (GetUserCount() > 0)
        return;

    MemoryModule::Get().Release();
    Module::Finalize();
    gFileModule = nullptr;
};

FileModule& FileModule::Get()
{
    return *gFileModule;
}

void FileModule::RegisterRoot(RootPathKey key, const Path::value_type* absPath)
{
    FileModulePrivateData* self = FileModuleImpl::GetCombinePtr(this);
    PMRAllocator           alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::SystemInit);
    self->RegisteredRoots.emplace(std::make_pair(key, Path(absPath, alloc)));
}

void FileModule::GetPath(Path& outPath, RootPathKey rootPath, const Path::value_type* relpath)
{
    FileModulePrivateData* self = FileModuleImpl::GetCombinePtr(this);
    PMRAllocator           alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::SystemInit);
    std::filesystem::path tpath = self->RegisteredRoots[rootPath];
    tpath /= relpath;
    outPath = tpath.wstring();
}

void FileModule::ReadFileContent(const Path& path, PMRVector<u8>& dst)
{
    std::ifstream ifile(path.c_str(), std::ios::binary | std::ios::ate);
    size_t sz = ifile.tellg();
    CheckAlways(sz != -1);
    ifile.seekg(0);
    size_t oldSize = dst.size();
    dst.resize(oldSize + sz);
    CheckAlways(ifile.read((char*)dst.data() + oldSize, sz));
}

EXPORT_INTERNAL_MODULE(File, ModuleExportInfo(FileModuleCtor, true));

} // namespace Omni
