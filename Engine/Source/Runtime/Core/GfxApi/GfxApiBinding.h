#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Core/GfxApi/GfxApiObjectHelper.h"

namespace Omni
{

// on GPU program argument/parameter binding
/*

# ideas of argument/parmeter orgnization
## DX11 style
- constants: 1 dict or 1 buffer/dict per stage
- SRV: (vs/ps) * (buffer/texture)

## MyIdea0
- allow overlap, prioritized structs
- struct
    - constants
    - buffers
    - textures
## MyIdead1
- no overlap, structs
- struct
    - constants
    - buffers
    - textures

# consideration
- accessiblity:
    - dx12, resource binding is specified once for PSO, so vs and ps do not have different binding slots, although can
speicify visibility
    - metal, set(Vertex/Fragment)(Buffer/Texture)
    - vulkan: resource binding is specified once for PSO, can have multiple descriptor sets, minimum 4 sets


# D3D12
## entities
- shader: declare used slot, looks like shader can't differentiate RootConstant from RootConstantBufferView, so it need
RootSignature
    - https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html#resource-binding-in-hlsl
    - example:
https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html#using-constants-directly-in-the-root-arguments
- RootSignature: declare API parameter slots, descriptor table/root descriptor describe 1. argument size and 2. how
arguments map to shader declaration
    - https://docs.microsoft.com/en-us/windows/win32/direct3d12/root-signatures
    - litmit and cost, 60dword max:https://docs.microsoft.com/en-us/windows/win32/direct3d12/root-signature-limits
    - speicify root signature in hlsl:
https://docs.microsoft.com/en-us/windows/win32/direct3d12/specifying-root-signatures-in-hlsl
- runtime binding:
    - https://docs.microsoft.com/en-us/windows/win32/direct3d12/using-a-root-signature
- promise descriptor heap content/root descriptor involiatile, optimization, root signature 1.1
    - https://docs.microsoft.com/en-us/windows/win32/direct3d12/root-signature-version-1-1

# vulkan
## entitites
- overview: https://vulkan.lunarg.com/doc/view/1.2.154.1/windows/tutorial/html/08-init_pipeline_layout.html
- DescriptorSet: similar to descriptor table
- DescriptorSetLayout: similar to part of a RootSignature
- PipelineLayout: array of DescriptorSetLayout
- constant data/uniform access easier methods: Uniform Buffer Dynamic Binding(buffer offset come from CommandBuffer
other than DescriptorSet), PushConstants(RootSignature constant)
- RenderPass/SubPass: the name RenderPass is inapproperiate, it should be called RenderGraph, SubPass should be called
RenderPass or RenderGraphNode

*/

/**
 * forward decls
 */
struct GfxApiBufferRef;

struct GfxApiBufferView
{
    GfxApiBufferRef Buffer;
    u32             Offset;
};

enum class GfxApiBindingType
{
    ConstantBuffer,
    Buffers,
    Textures,
    Samplers,
};

struct GfxApiBindingSlot
{
    GfxApiBindingType Type;
    u32               Space;
    u32               BaseRegister;
    u32               Range;
    u32               VisibleStageMask;
    u32               FromBindingGroup; //see also GfxApiBindingGroup
};

struct GfxApiPSOSignatureDesc // RootSignature in dx12, DescriptorSet in Vulkan, ArgumentBuffer in Metal
{
    GfxApiBindingSlot* Slots;
    u32                SlotCount;
};

struct GfxApiPurgePSOOptions
{
    u32 MinLastUsedFrames;
};


/**
 * typedefs
 */
DECLARE_GFXAPI_REF_TYPE(GfxApiPSOSignatureRef, RawPtrHandle);


} // namespace Omni
