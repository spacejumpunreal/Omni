#include "Runtime/Base/BasePCH.h"
#include "Runtime/Base/Memory/ObjectCache.h"

namespace Omni
{
	ObjectCacheBase::ObjectCacheBase(u32 nodeSize, u16 align, u16 growSize, PMRResource* memResource)
		: mResource(memResource)
		, mStack(nullptr)
		, mNodeAlign(align)
		, mNodeSize(nodeSize)
		, mGrowSize(growSize)
	{
	}
	ObjectCacheBase::~ObjectCacheBase()
	{}
	SListNode* ObjectCacheBase::AllocImpl()
	{
		if (mStack == nullptr)
		{
			size_t bytes = mGrowSize * mNodeSize;
			u8* ptr = (u8*)mResource->allocate(bytes, mNodeAlign);
			for (u16 iObj = 1; iObj < mGrowSize - 1; ++iObj)
			{
				SListNode* node = (SListNode*)ptr;
				node->Next = mStack;
				ptr += mNodeSize;
			}
		}
		SListNode* ret = mStack;
		mStack = mStack->Next;
		return ret;
	}
	void ObjectCacheBase::FreeImpl(SListNode* node)
	{
		node->Next = mStack;
		mStack = node;
	}
}