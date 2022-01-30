#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12Shader.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Text/TextEncoding.h"
#include "Runtime/Base/Memory/MemoryArena.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/OSUtils_Windows.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/DXCWrapper.h"

namespace Omni
{
/*
 * constants
 */
static const wchar_t* kDefaultBuildTarget[(u32)GfxApiShaderStage::Count] = {
    L"vs_6_0", // vertex
    L"ps_6_0", // fragment
    L"cs_6_0", // compute
};
static constexpr u32 kMaxShaderEntryLength = 64;

DX12Shader::DX12Shader(const GfxApiShaderDesc& desc) : mCompiledBinary(nullptr)
{
    if (!desc.Source.empty())
    {
        GfxApiShaderCompileOptions options;
        DX12ShaderCompileOptions   dx12Options;
#if OMNI_DEBUG
        dx12Options.Debug = true;
#else
        dx12Options.Debug = false;
#endif
        auto& stk = MemoryModule::GetThreadScratchStack();
        auto scope = stk.PushScope();
        char16_t* tmpEntryBuf = (char16_t*)stk.Allocate(sizeof(char16_t) * kMaxShaderEntryLength);
        FromUTF8ToUTF16(desc.EntryName, (size_t)-1, tmpEntryBuf, kMaxShaderEntryLength);
        dx12Options.EntryPoint = (wchar_t*)tmpEntryBuf;
        dx12Options.Target = kDefaultBuildTarget[(u32)desc.Stage];
        mCompiledBinary = gDX12GlobalState.DXCInstance->CompileShaderSource(desc.Source, options, dx12Options);
    }
    else
    {
        NotImplemented();
    }
}
DX12Shader::~DX12Shader()
{
    mCompiledBinary->Release();
}
ID3DBlob* DX12Shader::GetCompiledBinary()
{
    return mCompiledBinary;
}
} // namespace Omni

#endif
