#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12Shader.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/OSUtils_Windows.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "External/DirectXShaderCompiler/dxcapi.h"

namespace Omni
{
/*
 * constants
 */
static const char* kDefaultVSTarget = "vs_6_0";

DX12Shader::DX12Shader(const GfxApiShaderDesc& desc) : mCompiledBinary(nullptr)
{
    (void)desc;
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
