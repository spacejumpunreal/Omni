#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Memory/ObjectCache.h"
#include "Runtime/Core/Allocator/MemoryModule.h"


#define DECLARE_GFXAPI_OBJECT_CACHE_FACTORY_BEGIN(FactoryType, ObjectType) \
    struct FactoryType : public GfxApiObjectCacheFactory<ObjectType, FactoryType> \
    { \
        FactoryType(); \
        void* CreateObject() override; \
        void DestroyObject(void* obj) override; \
        void RecycleCleanup(void* obj) override;


#define DECLARE_GFXAPI_OBJECT_CACHE_FACTORY_END() };



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
