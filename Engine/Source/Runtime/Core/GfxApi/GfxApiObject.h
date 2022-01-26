#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Misc/EnumUtils.h"
#include "Runtime/Base/Misc/SharedObject.h"
#include "Runtime/Base/Memory/ObjectHandle.h"
#include "Runtime/Base/Math/Vector4.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/Platform/WindowUtils.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"

#include <variant>

namespace Omni
{
/**
 * forward decls
 */
// AsyncActions
class GfxApiBlitPass;
class GfxApiRenderPass;
class GfxApiComputePass;

/**
 * typedefs
 */
struct GfxApiTextureRef : public RawPtrHandle
{
    using UnderlyingHandle = RawPtrHandle;
};
struct GfxApiBufferRef : public RawPtrHandle
{
    using UnderlyingHandle = RawPtrHandle;
};
struct GfxApiSwapChainRef : public RawPtrHandle
{
    using UnderlyingHandle = RawPtrHandle;
};
struct GfxApiGpuEventRef : public RawPtrHandle
{
    using UnderlyingHandle = RawPtrHandle;
};

/**
 * enums
 */

/**
 * GfxApiObjectDesc definitions & GfxApiObject interface declarations
 */

// GfxApiObjectDesc
struct GfxApiObjectDesc
{
public:
    GfxApiObjectType Type;
    const char*      Name;

public:
    GfxApiObjectDesc(GfxApiObjectType type, const char* name = "") : Type(type), Name(name)
    {
    }
};


/**
 * Resources
 */

// GfxApiBuffer
struct GfxApiBufferDesc : public GfxApiObjectDesc
{
public:
    u32               Size = 0;
    u32               Align = 0;
    GfxApiAccessFlags AccessFlags;

public:
    GfxApiBufferDesc() : GfxApiObjectDesc(GfxApiObjectType::Buffer)
    {
    }
};

// GfxApiTexture
struct GfxApiTextureDesc : public GfxApiObjectDesc
{
public:
    u32               Width;
    u32               Height;
    GfxApiAccessFlags AccessFlags;
    GfxApiFormat      Format;

public:
    GfxApiTextureDesc() : GfxApiObjectDesc(GfxApiObjectType::Texture)
    {
    }
};

// GfxApiSwapChain
struct GfxApiSwapChainDesc : public GfxApiObjectDesc
{
public:
    u32          BufferCount;
    u32          Width;
    u32          Height;
    GfxApiFormat Format;
    WindowHandle WindowHandle;

public:
    GfxApiSwapChainDesc()
        : GfxApiObjectDesc(GfxApiObjectType::Swapchain), BufferCount(3), Width(0), Height(0),
          Format(GfxApiFormat::R8G8B8A8_UNORM)
    {
    }
};


/**
 * GPU program, PSO related
 */

// GfxApiShader
struct GfxApiShaderDesc
{
};

// GfxApiPSO
struct GGfxApiPSODesc
{
};

//on GPU program argument/parameter binding
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
    - dx12, resource binding is specified once for PSO, so vs and ps do not have different binding slots, although can speicify visibility
    - metal, set(Vertex/Fragment)(Buffer/Texture)
    - vulkan: resource binding is specified once for PSO, can have multiple descriptor sets, minimum 4 sets


# D3D12
## entities
- shader: declare used slot, looks like shader can't differentiate RootConstant from RootConstantBufferView, so it need RootSignature
    - https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html#resource-binding-in-hlsl
    - example: https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html#using-constants-directly-in-the-root-arguments
- RootSignature: declare API parameter slots, descriptor table/root descriptor describe 1. argument size and 2. how arguments map to shader declaration
    - https://docs.microsoft.com/en-us/windows/win32/direct3d12/root-signatures
    - litmit and cost, 60dword max:https://docs.microsoft.com/en-us/windows/win32/direct3d12/root-signature-limits
    - speicify root signature in hlsl: https://docs.microsoft.com/en-us/windows/win32/direct3d12/specifying-root-signatures-in-hlsl
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
- constant data/uniform access easier methods: Uniform Buffer Dynamic Binding(buffer offset come from CommandBuffer other than DescriptorSet), PushConstants(RootSignature constant)
- RenderPass/SubPass: the name RenderPass is inapproperiate, it should be called RenderGraph, SubPass should be called RenderPass or RenderGraphNode

*/



} // namespace Omni
