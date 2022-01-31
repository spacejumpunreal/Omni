#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DXCWrapper.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/OSUtils_Windows.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "External/DirectXShaderCompiler/dxcapi.h"

namespace Omni
{
/*
 * declaration
 */

struct DXCWrapperPrivateData
{
public:
    IDxcCompiler3* Compiler;
    IDxcUtils*     Utils;

public:
    DXCWrapperPrivateData();
    ~DXCWrapperPrivateData();
};

using DXCWrapperImpl = PImplCombine<DXCWrapper, DXCWrapperPrivateData>;

/*
 * definitions
 */

// DXCWrapper
DXCWrapper* DXCWrapper::Create()
{
    return OMNI_NEW(MemoryKind::GfxApi) DXCWrapperImpl();
}
void DXCWrapper::Destroy()
{
    DXCWrapperImpl* self = DXCWrapperImpl::GetCombinePtr(this);
    OMNI_DELETE(self, MemoryKind::GfxApi);
}
ID3DBlob* DXCWrapper::CompileShaderSource(std::string_view                  source,
                                      const GfxApiShaderCompileOptions& options,
                                      const DX12ShaderCompileOptions&   dx12Options)
{
    (void)options;
    DXCWrapperImpl* self = DXCWrapperImpl::GetCombinePtr(this);
    DxcBuffer       srcBuff = {
        .Ptr = source.data(),
        .Size = source.size(),
        .Encoding = 0,
    };
    PMRVector<const wchar_t*> args(MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi));
    args.reserve(32);
    args.push_back(L"-T");
    args.push_back(dx12Options.Target);
    args.push_back(L"-E");
    args.push_back(dx12Options.EntryPoint);
    if (dx12Options.Debug)
    {
        args.push_back(L"-Zi");
        args.push_back(L"-O0");
    }
    else
    {
        args.push_back(L"-O3");
    }
    IDxcResult*   result;
    IDxcBlob*     rBlob;
    IDxcBlobUtf8* errors;

    CheckDX12(self->Compiler->Compile(&srcBuff, args.data(), (u32)args.size(), nullptr, IID_PPV_ARGS(&result)));
    CheckDX12(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
    if (errors && errors->GetStringLength() > 0)
    {
        const char* msg = errors->GetStringPointer();
        CheckAlways(false, msg);
    }
    CheckDX12(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&rBlob), nullptr));
    result->Release();
    return (ID3DBlob*)rBlob;
}

// DXCWrapperPrivateData
DXCWrapperPrivateData::DXCWrapperPrivateData()
{
    CheckDX12(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&Utils)));
    CheckDX12(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler)));
}

DXCWrapperPrivateData::~DXCWrapperPrivateData()
{
    Utils->Release();
    Compiler->Release();
}

} // namespace Omni

#endif
