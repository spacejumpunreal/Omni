#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Base/Container/LinkedList.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Base/Memory/PageSubAllocator.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Core/GfxApi/GfxApiGraphicState.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/GfxApiBinding.h"

#include <variant>

namespace Omni
{
/**
 * forward decls
 */
struct GfxApiBindingGroup;
struct GfxApiViewport;

/**
 * typedefs
 */
using TClearValue = std::variant<u32, u8, Vector4, float>;

/**
 * enums
 */

/**
 * definitions
 */

struct GfxApiBindingGroup
{
    GfxApiBufferView  ConstantBuffer;
    GfxApiBufferRef*  Buffers;
    u32               BufferCount;
    GfxApiTextureRef* Textures;
    u32               TextureCount;
};

struct GfxApiDirectDrawParams
{
    u32 IndexCount;
    u32 InstanceCount;
    u32 FirstIndex; // offset(not byte) of first index element(16bit or 32bit)
    u32 BaseVertex;
    u32 BaseInstance;
};

struct GfxApiIndirectDrawParams
{
    GfxApiBufferRef IndirectArgBuffer;
    u32             ArgOffset; // in bytes
};

struct GfxApiPSOParams
{
    GfxApiPSOSignatureRef      Signature;
    GfxApiShaderRef            Shaders[(u32)GfxApiShaderStage::GraphicsCount];
    GfxApiBlendStateRef        BlendState;
    GfxApiRasterizerStateRef   RasterizerState;
    GfxApiDepthStencilStateRef DepthStencilState;
};

struct GfxApiDrawcall
{
    // Drawcall Api
    GfxApiBufferRef IndexBuffer;
    union
    {
        GfxApiDirectDrawParams DirectDrawArgs;
        GfxApiDirectDrawParams IndirectDrawArgs;
    } DrawArgs;

    // Binding/Arguments
    GfxApiBindingGroup* BindingGroups[(u8)GfxApiBindingGroupSlot::Count];

    GfxApiPSOParams PSOParams;
    GfxApiViewport* Viewports;
    u8              StencilRef;

    // flags
    bool IsIndirect;
    bool Is32BitIndexBuffer;
};

struct GfxApiRenderPassStage : SListNode
{
public:
    CORE_API GfxApiRenderPassStage(PageSubAllocator* alloc, u32 capacity);
    ~GfxApiRenderPassStage() = delete;

public:
    GfxApiDrawcall* Drawcalls;
    u32             DrawcallCount;
};

struct GfxApiRTConfig
{
    GfxApiTextureRef       Texture = {};
    TClearValue            ClearValue;
    GfxApiLoadStoreActions Action = GfxApiLoadStoreActions::Clear | GfxApiLoadStoreActions::Store;
};

class GfxApiRenderPass
{
public:
    CORE_API GfxApiRenderPass(PageSubAllocator* alloc, u32 PhaseCount);
    ~GfxApiRenderPass() = delete;
    CORE_API void AddStage(u32 stageIndex, GfxApiRenderPassStage* passStage);
    CORE_API u32  GetStageCount() const;

public:
    GfxApiRTConfig RenderTargets[kMaxMRTCount];
    GfxApiRTConfig Depth;
    GfxApiRTConfig Stencil;

public:
    GfxApiRenderPassStage** PhaseArray;
    u32                     PhaseCount;
};

struct GfxApiViewport
{
    float TopLeftX;
    float TopLeftY;
    float Width;
    float Height;
    float MinDepth;
    float MaxDepth;
};

} // namespace Omni
