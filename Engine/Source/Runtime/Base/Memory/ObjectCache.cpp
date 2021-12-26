#include "Runtime/Base/BasePCH.h"
#include "Runtime/Base/Memory/ObjectCache.h"
#include "Runtime/Base/Misc/AssertUtils.h"

namespace Omni
{
    void ObjectCacheBase::Initialize(PMRAllocator allocator, IObjectCacheFactory* factory, u32 growCount)
    {
        mFactory = factory;
        mGrowCount = growCount;
        CheckAlways(mObjects.size() == 0);
        (&mObjects)->~PMRVector<void*>();
        new (&mObjects)PMRVector<void*>(allocator.resource());
    }
    ObjectCacheBase::~ObjectCacheBase()
    {
        Cleanup();
        mFactory->Destroy();
    }
    void* ObjectCacheBase::Alloc()
    {
        if (mObjects.size() == 0)
        {
            mObjects.reserve(mObjects.size() + mGrowCount);
            for (u32 i = 0; i < mGrowCount; ++i)
                mObjects.emplace_back(mFactory->CreateObject());
        }
        void* ret = mObjects.back();
        mObjects.pop_back();
        return ret;

    }
    void ObjectCacheBase::Free(void* obj)
    {
        mObjects.push_back(obj);
    }
    void ObjectCacheBase::Cleanup()
    {
        for (void* obj : mObjects)
        {
            mFactory->DestroyObject(obj);
        }
        mObjects.clear();
    }
}
