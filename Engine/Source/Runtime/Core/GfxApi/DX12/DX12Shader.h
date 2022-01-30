#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/DX12/DX12Basics.h"
#include "Runtime/Core/GfxApi/DX12/DX12ForwardDecl.h"

namespace Omni
{
class DX12Shader
{
public:
    DX12Shader(const GfxApiShaderDesc& desc);
    ~DX12Shader();
    ID3DBlob* GetCompiledBinary();

private:
    ID3DBlob* mCompiledBinary;
};
} // namespace Omni

#endif // OMNI_WINDOWS
