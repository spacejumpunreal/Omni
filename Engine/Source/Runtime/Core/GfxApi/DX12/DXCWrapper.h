#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/GfxApiShader.h"
#include "Runtime/Core/GfxApi/DX12/DX12Basics.h"
#include "Runtime/Core/GfxApi/DX12/DX12ForwardDecl.h"

struct ID3D10Blob;
namespace Omni
{
struct DX12ShaderCompileOptions
{
    const wchar_t* Target;
    const wchar_t* EntryPoint;
    bool           Debug;
};

class DXCWrapper
{
public:
    static DXCWrapper* Create();
    void               Destroy();

    ID3DBlob* CompileShaderSource(std::string_view                  source,
                              const GfxApiShaderCompileOptions& options,
                              const DX12ShaderCompileOptions&   dx12Options);
};

} // namespace Omni

#endif // OMNI_WINDOWS
