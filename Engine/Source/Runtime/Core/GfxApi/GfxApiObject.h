#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Misc/EnumUtils.h"
#include "Runtime/Base/Misc/SharedObject.h"
#include "Runtime/Base/Math/Vector4.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/Platform/WindowUtils.h"
#include <variant>


namespace Omni
{
    /**
     * forward decls
     */
    class GfxApiTexture;
    class GfxApiCommandContext; //for threaded recording
    class GfxApiRenderPass;
    class GfxApiSwapChain;

    /**
     * enums
     */
    enum class GfxApiObjectType : u32
    {
        Buffer,
        Texture,
        Sampler,
        Pso,
        Shader,
        Fence,
        Swapchain,
    };

    enum class GfxApiFormat : u32
    {
        R8G8B8A8_UNORM,
        R16G16B16A16_FLOAT,
        R11G11B10_FLOAT,
    };

    enum class GfxApiAccessFlags : u32
    {
        CPUAccess = 1 << 0,
        GPURead = 1 << 1,
        GPUWrite = 1 << 2,
    };

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
        virtual const GfxApiSwapChainDesc& GetDesc() = 0;
        virtual void Present(bool waitFotVSync) = 0;
        virtual void Update(const GfxApiSwapChainDesc& desc) = 0;
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
        Store = 1 << 2,
        Discard = 1 << 3,
    };
    DEFINE_ENUM_CLASS_OPS(GfxApiLoadStoreActions)

    struct GfxApiRTConfig
    {
        SharedPtr<GfxApiTexture>    Texture;
        std::variant<u32, Vector4>  ClearValue;
        GfxApiLoadStoreActions      Action = GfxApiLoadStoreActions::Clear | GfxApiLoadStoreActions::Store;

    };

    constexpr u32 MaxMRTCount = 8;
    struct GfxApiRenderPassDesc
    {
        GfxApiRTConfig      Color[MaxMRTCount];
        GfxApiRTConfig      Depth;
        GfxApiRTConfig      Stencil;
    };

    struct GfxApiCommandContextDesc
    {
        GfxApiContextType   Type;
        GfxApiRenderPass*   RenderPass;
    };

    class GfxApiRenderPass
    {
    public:
    };

    class GfxApiCommandContext
    {
    public:
    };
}