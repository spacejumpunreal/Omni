#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Core/System/Module.h"
#include <filesystem>

namespace Omni
{
using Path = PMRUTF16String; // sadly std::filesystem::path do not support PMR, so we use utf8string
using RootPathKey = u32;

struct PredefinedPath
{
    static constexpr u32 ProjectRoot = 0;
};

class CORE_API FileModule : public Module
{
public:
    void Destroy() override;
    void Initialize(const EngineInitArgMap&) override;
    void Finalize() override;

    static FileModule& Get();

    void RegisterRoot(RootPathKey key, const Path::value_type* absPath);
    void GetPath(Path& outPath, RootPathKey rootPath, const Path::value_type* relpath);

    void ReadFileContent(const Path& path, PMRVector<u8>& dst);
};

} // namespace Omni
