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
    //GfxApiObejcts
    class GfxApiBuffer;
    class GfxApiTexture;
    class GfxApiSwapChain;
    class GfxApiGpuEvent;
    //AsyncActions
    class GfxApiRenderPass;
    class GfxApiComputePass;

    /**
     * typedefs
     */
    using GfxApiTextureRef = GfxApiTexture*;
    using GfxApiBufferRef = GfxApiBuffer*;
    struct GfxApiSwapChainRef : public IndexHandle {};
    using GfxApiGpuEventRef = GfxApiGpuEvent*;

    /**
     * enums
     */


    /**
     * GfxApiObjectDesc definitions & GfxApiObject interface declarations
     */
    
    //GfxApiObjectDesc
    struct GfxApiObjectDesc
    {
    public:
        GfxApiObjectType Type;
        const char* Name;
    public:
        GfxApiObjectDesc(GfxApiObjectType type, const char* name = "") 
            : Type(type) 
            , Name(name)
        {}
    };


    //GfxApiBuffer
    struct GfxApiBufferDesc : public GfxApiObjectDesc
    {
    public:
        u32                 Size;
        GfxApiAccessFlags   AccessFlags;

    public:
        GfxApiBufferDesc() : GfxApiObjectDesc(GfxApiObjectType::Buffer) {}
    };

    class GfxApiBuffer
    {
        virtual const GfxApiBufferDesc& GetDesc() = 0;
    };


    //GfxApiTexture
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

    class GfxApiTexture
    {
    public:
        virtual const GfxApiTextureDesc& GetDesc() = 0;
    };


    //GfxApiSwapChain
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

    class GfxApiSwapChain
    {
    public:
        virtual const GfxApiSwapChainDesc& GetDesc() = 0;
    };
    

    //GfxApiGpuEvent
    class GfxApiGpuEvent //aka ID3D12Fence on DX12, MTLEvent on Metal
    {
    public:
    };
}
