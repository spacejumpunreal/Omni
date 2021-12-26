#include "Runtime/Base/BasePCH.h"
#include "Runtime/Base/Memory/ObjectCache.h"
#include "Runtime/Base/Misc/AssertUtils.h"

namespace Omni
{
    ObjectCacheBase::ObjectCacheBase()
        : mAllocCount(0)
    {}
    void ObjectCacheBase::Initialize(PMRAllocator allocator, IObjectCacheFactory* factory, u32 growCount)
    {
        CheckAlways(mObjects.size() == 0 && mAllocCount == 0);
        (&mObjects)->~PMRVector<void*>();
        mFactory = factory;
        mGrowCount = growCount;
        new (&mObjects)PMRVector<void*>(allocator.resource());
    }
    ObjectCacheBase::~ObjectCacheBase()
    {
        CheckAlways(mAllocCount == mObjects.size(), "not all objects returned to cache before destroying cache");
        Cleanup();
    }
    void* ObjectCacheBase::Alloc()
    {
        if (mObjects.size() == 0)
        {
            mObjects.reserve(mObjects.size() + mGrowCount);
            for (u32 i = 0; i < mGrowCount; ++i)
                mObjects.emplace_back(mFactory->CreateObject());
            mAllocCount += mGrowCount;
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
        mAllocCount = 0;
        mObjects.clear();
        if (mFactory)
        {
            mFactory->Destroy();
            mFactory = nullptr;
        }
            
    }
}
