#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12Shader.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Text/TextEncoding.h"
#include "Runtime/Base/Memory/MemoryArena.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/OSUtils_Windows.h"
#include "Runtime/Core/GfxApi/GfxApiBinding.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/DXCWrapper.h"
#include "Runtime/Core/GfxApi/DX12/d3dx12.h"

namespace Omni
{
/*
 * constants
 */
static const wchar_t* kDefaultBuildTarget[(u32)GfxApiShaderStage::TotalCount] = {
    L"vs_6_0", // vertex
    L"ps_6_0", // fragment
    L"cs_6_0", // compute
};
static constexpr u32 kMaxShaderEntryLength = 64;

/*
 * definitions
 */

// DX12Shader
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
        auto&     stk = MemoryModule::GetThreadScratchStack();
        auto      scope = stk.PushScope();
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
    SafeRelease(mCompiledBinary);
}
ID3DBlob* DX12Shader::GetCompiledBinary()
{
    return mCompiledBinary;
}

// DX12PSOSignature
static D3D12_SHADER_VISIBILITY CalcShaderVisibility(u32 mask)
{
    u32 acc = 0;
    if (mask & (1 << u32(GfxApiShaderStage::Vertex)))
    {
        acc |= D3D12_SHADER_VISIBILITY_VERTEX;
    }
    if (mask & (1 << u32(GfxApiShaderStage::Fragment)))
    {
        acc |= D3D12_SHADER_VISIBILITY_PIXEL;
    }
    return D3D12_SHADER_VISIBILITY(acc);
}

DX12PSOSignature::DX12PSOSignature(const GfxApiPSOSignatureDesc& desc) : mSlotCount(desc.SlotCount)
{
    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootDesc;
    D3D12_ROOT_SIGNATURE_FLAGS            flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    auto&                    stk = MemoryModule::Get().GetThreadScratchStack();
    auto                     scope = stk.PushScope();
    CD3DX12_ROOT_PARAMETER1* params = stk.AllocArray<CD3DX12_ROOT_PARAMETER1>(desc.SlotCount);
    for (u32 iSlot = 0; iSlot < desc.SlotCount; ++iSlot)
    {
        GfxApiBindingSlot& slotDef = desc.Slots[iSlot];
        switch (desc.Slots[iSlot].Type)
        {
        default:
        case GfxApiBindingType::ConstantBuffer:
            params[iSlot].InitAsConstantBufferView(slotDef.BaseRegister,
                slotDef.Space,
                D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
                CalcShaderVisibility(slotDef.VisibleStageMask));
            break;
        case GfxApiBindingType::Buffers:
            D3D12_DESCRIPTOR_RANGE1 range;
            range.BaseShaderRegister = slotDef.BaseRegister;
            // range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;
            range.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC |
                          D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_STATIC_KEEPING_BUFFER_BOUNDS_CHECKS;
            range.NumDescriptors = slotDef.Range;
            range.OffsetInDescriptorsFromTableStart = 0;
            range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
            range.RegisterSpace = slotDef.Space;
            params[iSlot].InitAsDescriptorTable(1, &range, CalcShaderVisibility(slotDef.VisibleStageMask));
            break;
        case GfxApiBindingType::Textures:
            NotImplemented();
            break;
        case GfxApiBindingType::Samplers:
            NotImplemented();
            break;
        }
    }
    rootDesc.Init_1_1(desc.SlotCount, params, 0, nullptr, flags);
    ID3DBlob* blob{};
    ID3DBlob* error{};
    HRESULT   hr = D3DX12SerializeVersionedRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &blob, &error);
    if (error)
    {
        CheckAlways(false, error->GetBufferPointer());
    }
    CheckDX12(hr);
    if (error)
        error->Release();
    gDX12GlobalState.Singletons.D3DDevice->CreateRootSignature(0,
        blob->GetBufferPointer(),
        blob->GetBufferSize(),
        IID_PPV_ARGS(&mRootSignature));
    if (blob)
        blob->Release();
    mSlots = MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi).allocate_object<GfxApiBindingSlot>(mSlotCount);
    memcpy(mSlots, desc.Slots, sizeof(GfxApiBindingSlot) * mSlotCount);
}
DX12PSOSignature::~DX12PSOSignature()
{
    SafeRelease(mRootSignature);
    MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi).deallocate_object<GfxApiBindingSlot>(mSlots, mSlotCount);
}
ID3D12RootSignature* DX12PSOSignature::GetRootSignature()
{
    return mRootSignature;
}
u32 DX12PSOSignature::GetRootParamCount()
{
    return mSlotCount;
}
const GfxApiBindingSlot* DX12PSOSignature::GetRootParamDescs()
{
    return mSlots;
}
} // namespace Omni

#endif
