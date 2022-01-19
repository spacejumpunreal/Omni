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

struct DirectDrawParams
{
    u32 IndexCount;
    u32 InstanceCount;
    u32 FirstIndex;
    u32 BaseVertex;
    u32 BaseInstance;
};

struct IndirectDrawParams
{
    GfxApiBufferRef IndirectArgBuffer;
};

struct GfxApiDrawcall
{
    GfxApiBufferRef  IndexBuffer = (GfxApiBufferRef)NullPtrHandle;
    DirectDrawParams DrawParams;
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
