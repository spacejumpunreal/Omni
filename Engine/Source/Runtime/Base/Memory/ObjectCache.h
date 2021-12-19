#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Memory/MemoryDefs.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Base/Container/LinkedList.h"

namespace Omni
{
	class BASE_API ObjectCacheBase
	{
	public:
	protected:
		ObjectCacheBase(u32 nodeSize, u16 align, u16 growSize, PMRResource* memResource);
		~ObjectCacheBase();
		SListNode* AllocImpl();
		void FreeImpl(SListNode*);
	private:
		PMRResource*	mResource;
		SListNode*		mStack;
		u32				mNodeSize;
		u16				mNodeAlign;
		u16				mGrowSize;
	};

	template<typename T>
	concept ReusableObject = requires
		(T a) { a.Cleanup(); T{}; }	&& 
		sizeof(T) >= sizeof(void*);

	template<ReusableObject TObject>
	class ObjectCache : ObjectCacheBase
	{
	public:
		TObject* Alloc()
		{ 
			TObject* ret = (TObject*)AllocImpl();
			return ret;
		}
		void Free(TObject* obj) 
		{ 
			obj->Cleanup();
			FreeImpl((SListNode*)obj);
		}
	public:
		ObjectCache(PMRResource* memResource, u32 growSize)
			: ObjectCacheBase(
				AlignUpSize(sizeof(TObject), alignof(TObject)),
				alignof(TObject), growSize,
				memResource)
		{}
	};

	
}