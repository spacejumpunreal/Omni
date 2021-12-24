#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Memory/MemoryDefs.h"
#include "Runtime/Base/Misc/ArrayUtils.h"

namespace Omni
{
	template<typename T>
	concept ReusableObject = requires
		(T a) { a.CleanupForRecycle(); T{}; } &&
		sizeof(T) >= sizeof(void*);

    //TODO: add refiller, make it virtual to avoid dependency
	template<ReusableObject TObject>
	class ObjectCache
	{
	private:
		struct TObjectNode
		{
			TObject Object;
			TObjectNode* Next;
		};
	public:
		ObjectCache(PMRResource* resource, u32 growCount)
			: mStack(nullptr)
			, mGrowCount(growCount)
			, mAllocator(resource)
		{}
		~ObjectCache()
		{
			Cleanup();
		}
		TObject* Alloc()
		{
			if (mStack)
			{
				TObjectNode* ret = mStack;
				mStack = mStack->Next;
				return (TObject*)ret;
			}
			TObjectNode* node = mAllocator.new_object<TObjectNode>();
			return (TObject*)node;
		}
		void Free(TObject* obj) 
		{
			obj->CleanupForRecycle();
			TObjectNode* node = (TObjectNode*)obj;
			node->Next = mStack;
			mStack = node;
		}
		void Cleanup()
		{
			while (mStack)
			{
				TObjectNode* node = mStack;
				mStack = mStack->Next;
				TObjectNode* obj = (TObjectNode*)node;
				mAllocator.delete_object(obj);
			}
		}
	private:
		TObjectNode* mStack;
		u32 mGrowCount;
		PMRAllocatorT<TObjectNode> mAllocator;
	};
}
