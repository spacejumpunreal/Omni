#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DXCWrapper.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Misc/PImplUtils.h"
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
void* DXCWrapper::CompileShaderSource(std::string_view source)
{
    (void)source;
    return nullptr;
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
