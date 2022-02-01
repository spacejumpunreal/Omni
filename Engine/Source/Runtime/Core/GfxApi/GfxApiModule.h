#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Base/Misc/SharedObject.h"
#include "Runtime/Core/System/Module.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Core/GfxApi/GfxApiBinding.h"

namespace Omni
{
// this is just an interface, no GfxApiModule interface implementation will actually be provided, derived class like
// DX12Module will actually provide it
class CORE_API GfxApiModule : public Module
{
public:
    static GfxApiModule& Get()
    {
        return *gGfxApiModule;
    }

#define GfxApiMethod(Definition) virtual Definition = 0;
#include "Runtime/Core/GfxApi/GfxApiMethodList.inl"
#undef GfxApiMethod

protected:
    static GfxApiModule* gGfxApiModule;
};
} // namespace Omni
