#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Memory/MemoryDefs.h"
#include "Runtime/Base/Container/PMRContainers.h"

namespace Omni
{
    struct IObjectCacheFactory
    {
        virtual void* CreateObject() = 0;
        virtual void DestroyObject(void* obj) = 0;
        virtual void RecycleCleanup(void* obj)  = 0;
        virtual void Destroy() = 0;
    };

    struct ObjectCacheBase
    {
    public:
        BASE_API void Initialize(PMRAllocator allocator, IObjectCacheFactory* factory, u32 growCount);
        BASE_API ObjectCacheBase();
        BASE_API ~ObjectCacheBase();
        BASE_API void* Alloc();
        BASE_API void Free(void* obj);
        BASE_API void Cleanup();
    protected:
        PMRVector<void*>            mObjects;
        IObjectCacheFactory*        mFactory;
        u32                         mGrowCount;
        u32                         mAllocCount;
    };

	template<typename TObject>
	class ObjectCache final : public ObjectCacheBase
	{
	public:
        ObjectCache(PMRAllocator allocator = {}, IObjectCacheFactory* factory = nullptr, u32 growCount = 1)
		{
            Initialize(allocator, factory, growCount);
        }
		TObject* Alloc()
		{
            return (TObject*)ObjectCacheBase::Alloc();
		}
		void Free(TObject* obj) 
		{
            ObjectCacheBase::Free(obj);
		}
	};
}
