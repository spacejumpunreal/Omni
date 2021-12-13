#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Misc/EnumUtils.h"
#include "Runtime/Base/Misc/SharedObject.h"
#include "Runtime/Core/CoreAPI.h"


namespace Omni
{
    /**
     * forward decls
     */
    class GfxApiTexture;
    class GfxApiContext; //for threaded recording
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
        GPUAccess = 1 << 1,
    };

    /**
     * GfxApiObjectDesc definitions & GfxApiObject interface declarations
     */
    struct GfxApiObjectDesc
    {
    protected:
        GfxApiObjectType Type;
    public:
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


    struct GfxApiSwapChainDesc : public GfxApiObjectDesc
    {
    public:
        u32                 BufferCount;
        u32                 Width;
        u32                 Height;
        GfxApiFormat        Format;
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
        virtual void Present() = 0;
        virtual SharedPtr<GfxApiTexture> GetCurrentBackbuffer() = 0;
    };


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
        DontCare = 1 << 3,
    };
    DEFINE_ENUM_CLASS_OPS(GfxApiLoadStoreActions)

    struct GfxApiRTConfig
    {
        SharedPtr<GfxApiTexture>    Texture;
        GfxApiLoadStoreActions      Action = GfxApiLoadStoreActions::Clear | GfxApiLoadStoreActions::DontCare;

    };

    constexpr u32 MaxMRTCount = 8;
    struct GfxApiRenderPassDesc
    {
        GfxApiRTConfig      Color[MaxMRTCount];
        GfxApiRTConfig      Depth;
        GfxApiRTConfig      Stencil;
    };

    struct GfxApiContextDesc
    {
        GfxApiContextType   Type;
        GfxApiRenderPass*   RenderPass;
    };
}