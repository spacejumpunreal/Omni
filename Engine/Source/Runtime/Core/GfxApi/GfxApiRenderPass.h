#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Container/LinkedList.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"

#include <variant>

namespace Omni
{
/**
 * forward decls
 */

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

struct GfxApiBufferView
{
    GfxApiBufferRef Buffer;
    u32             Offset;
};

struct GfxApiShaderArgGroup
{
    GfxApiBufferView ConstantBuffer;
    GfxApiBufferRef* BufferRefs;
    u32              BufferCount;
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

struct GfxApiDrawcall
{
    // Drawcall Api
    GfxApiBufferRef IndexBuffer = (GfxApiBufferRef)NullPtrHandle;
    union
    {
        GfxApiDirectDrawParams DirectDrawArgs;
        GfxApiDirectDrawParams IndirectDrawArgs;
    } DrawArgs;
    // Binding
    GfxApiShaderArgGroup* ArgGroups[(u8)GfxApiShaderArgGroupSlot::Count];

    // flags
    bool IsIndirect = false;
};

struct GfxApiRenderPassStage : SListNode
{
    DEFINE_GFX_API_TEMP_NEW_DELETE()
    PMRDeque<GfxApiDrawcall> Drawcalls;
};

struct GfxApiRTConfig
{
    GfxApiTextureRef       Texture = (GfxApiTextureRef)NullPtrHandle;
    TClearValue            ClearValue;
    GfxApiLoadStoreActions Action = GfxApiLoadStoreActions::Clear | GfxApiLoadStoreActions::Store;
};

class GfxApiRenderPass
{
public:
    DEFINE_GFX_API_TEMP_NEW_DELETE()
    CORE_API GfxApiRenderPass(u32 stageCount);
    CORE_API ~GfxApiRenderPass();
    CORE_API void AddStage(u32 stageIndex, GfxApiRenderPassStage* passStage);

public:
    GfxApiRTConfig RenderTargets[MaxMRTCount];
    GfxApiRTConfig Depth;
    GfxApiRTConfig Stencil;

private:
    GfxApiRenderPassStage** mStages;
    u32                     mStageCount;
};

} // namespace Omni
