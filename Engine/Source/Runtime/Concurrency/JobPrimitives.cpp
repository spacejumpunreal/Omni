#include "Runtime/Concurrency/JobPrimitives.h"
#include "Runtime/Memory/MemoryModule.h"

namespace Omni
{
	using UnaryFunctionType = void (*)(void*);

	void DispatchWorkItem::Perform()
	{
		auto f = (UnaryFunctionType)mFPtr;
		void* ap = GetArgPtr(this);
		f(ap);
	}
	void DispatchWorkItem::Destroy()
	{
		this->~DispatchWorkItem();
		PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::CacheLine);
		alloc.resource()->deallocate(this, 0);
	}
	DispatchWorkItem& DispatchWorkItem::CreateImpl(void* f, size_t aSize)
	{
		PMRAllocator alloc = MemoryModule::Get().GetPMRAllocator(MemoryKind::CacheLine);
		DispatchWorkItem* ret = (DispatchWorkItem*)alloc.resource()->allocate(sizeof(DispatchWorkItem) + aSize);
		new (ret)DispatchWorkItem();
		ret->mFPtr = f;
		return *ret;
	}
}