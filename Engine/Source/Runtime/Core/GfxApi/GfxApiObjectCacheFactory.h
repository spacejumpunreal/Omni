#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Memory/ObjectCache.h"
#include "Runtime/Core/Allocator/MemoryModule.h"

namespace Omni
{
    template<typename TObject, typename FactoryType>
    struct GfxApiObjectCacheFactory : public IObjectCacheFactory
    {
        void* operator new(size_t)
        {
            return MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi).allocate_bytes(sizeof(TObject), alignof(TObject));
        }
        void Destroy() override
        {
            MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi).delete_object(static_cast<FactoryType*>(this));
        }
    };

}
