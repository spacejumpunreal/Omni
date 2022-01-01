#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Misc/EnumUtils.h"
#include "Runtime/Base/Misc/SharedObject.h"
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
    class GfxApiTexture;
    class GfxApiSwapChain;
    class GfxApiRenderPass;
    class GfxApiRenderCommandContext;
    class GfxApiGpuEvent;

    /**
     * typedefs
     */
    using GfxApiGpuEventRef = SharedPtr<GfxApiGpuEvent>;


    /**
     * enums
     */


    /**
     * GfxApiObjectDesc definitions & GfxApiObject interface declarations
     */

    struct GfxApiObjectDesc
    {
    public:
        const GfxApiObjectType Type;
        const char* Name;
    public:
        GfxApiObjectDesc(GfxApiObjectType type, const char* name = "") 
            : Type(type) 
            , Name(name)
        {}
    };

    struct GfxApiBufferDesc : public GfxApiObjectDesc
    {
    public:
        u32                 Size;
        GfxApiAccessFlags   AccessFlags;

    public:
        GfxApiBufferDesc() : GfxApiObjectDesc(GfxApiObjectType::Buffer) {}
    };

    class GfxApiBuffer : public SharedObject
    {
        virtual const GfxApiBufferDesc& GetDesc() = 0;
    };

    using GfxApiBufferRef = SharedPtr<GfxApiBuffer>;

    struct GfxApiTextureDesc : public GfxApiObjectDesc
    {
    public:
        u32                 Width;
        u32                 Height;
        GfxApiAccessFlags   AccessFlags;
        GfxApiFormat        Format;

    public:
        GfxApiTextureDesc() : GfxApiObjectDesc(GfxApiObjectType::Texture) {}
    };

    class GfxApiTexture : public SharedObject
    {
    public:
        virtual const GfxApiTextureDesc& GetDesc() = 0;
    };

    using GfxApiTextureRef = SharedPtr<GfxApiTexture>;

    struct GfxApiSwapChainDesc : public GfxApiObjectDesc
    {
    public:
        u32                 BufferCount;
        u32                 Width;
        u32                 Height;
        GfxApiFormat        Format;
        WindowHandle        WindowHandle;
    public:
        GfxApiSwapChainDesc() : 
            GfxApiObjectDesc(GfxApiObjectType::Swapchain)
            , BufferCount(3)
            , Width(0)
            , Height(0)
            , Format(GfxApiFormat::R8G8B8A8_UNORM)
        {}
    };


    class GfxApiSwapChain : public SharedObject
    {
    public:
        struct FrameStatistics
        {
            u64     PresentedFrameCount;
            u64     CurrentFrameIndex;
        };
    public:
        virtual const GfxApiSwapChainDesc& GetDesc() = 0;
        virtual GfxApiGpuEvent Present(bool waitFotVSync) = 0;
        virtual void Update(const GfxApiSwapChainDesc& desc) = 0;
        virtual void Stats(FrameStatistics& stats) = 0;
        virtual u32 GetCurrentBackbufferIndex() = 0;
        virtual GfxApiTextureRef GetCurrentBackbuffer() = 0;
    };
    
    using GfxApiSwapChainRef = SharedPtr<GfxApiSwapChain>;


    /**
      * Command related
      */

    enum class GfxApiContextType
    {
        Copy,
        Compute,
        Render,
    };

    enum class GfxApiLoadStoreActions : u32
    {
        Load = 1 << 0,
        Clear = 1 << 1,
        DontCare = 1 << 2,
        Store = 1 << 3,
        Discard = 1 << 4,
    };
    DEFINE_ENUM_CLASS_OPS(GfxApiLoadStoreActions)

    struct GfxApiRTConfig
    {
        GfxApiTexture*              Texture = nullptr;
        std::variant<u32, Vector4>  ClearValue;
        GfxApiLoadStoreActions      Action = GfxApiLoadStoreActions::Clear | GfxApiLoadStoreActions::Store;

    };

    constexpr u32 MaxMRTCount = 8;
    struct GfxApiRenderPassDesc
    {
        GfxApiRTConfig      Color[MaxMRTCount];
        GfxApiRTConfig      Depth;
        GfxApiRTConfig      Stencil;
        u32                 StageCount = 1;
    };

    class GfxApiRenderPass
    {
    public:
        virtual GfxApiRenderCommandContext* BeginContext(u32 phase) = 0;
        virtual void EndContext(GfxApiRenderCommandContext* ctx) = 0;
    };

    class GfxApiRenderCommandContext
    {
    public:
        virtual void Use() = 0;
    };

    class GfxApiGpuEvent : public SharedObject //aka ID3D12Fence on DX12, MTLEvent on Metal
    {
    public:
        virtual bool IsDone() = 0;
        virtual void Wait() = 0;
    };
}
