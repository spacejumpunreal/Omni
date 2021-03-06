#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/DX12/DX12Basics.h"
#include "Runtime/Core/GfxApi/DX12/DX12ForwardDecl.h"

namespace Omni
{
struct GfxApiShaderDesc;
struct GfxApiPSOSignatureDesc;
struct GfxApiBindingSlot;

class DX12Shader
{
public:
    DX12Shader(const GfxApiShaderDesc& desc);
    ~DX12Shader();
    ID3D10Blob* GetCompiledBinary();

private:
    ID3D10Blob* mCompiledBinary;
};

class DX12PSOSignature
{
public:
    DX12PSOSignature(const GfxApiPSOSignatureDesc& desc);
    ~DX12PSOSignature();
    ID3D12RootSignature*     GetRootSignature();
    u32                      GetRootParamCount();
    const GfxApiBindingSlot* GetRootParamDescs();

private:
    ID3D12RootSignature* mRootSignature;
    GfxApiBindingSlot*   mSlots;
    u32                  mSlotCount;
};

} // namespace Omni

#endif // OMNI_WINDOWS
