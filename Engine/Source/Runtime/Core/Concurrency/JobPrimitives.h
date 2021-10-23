#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Base/Container/LinkedList.h"
#include "Runtime/Base/Memory/MemoryDefs.h"
#include "Runtime/Base/Misc/PrivateData.h"
#include "Runtime/Base/MultiThread/IShared.h"
#include "Runtime/Base/MultiThread/LockQueue.h"
#include "Runtime/Prelude/PlatformDefs.h"



namespace Omni
{
	class DispatchWorkItem : public SListNode
	{
	public:
		template<typename T>
		static DispatchWorkItem& Create(void (*func)(T*), const T* argPtr, MemoryKind memKind)
		{
			static_assert(std::is_standard_layout_v<T> && std::is_trivial_v<T>);
			static_assert(alignof(DispatchWorkItem) <= sizeof(void*));
			DispatchWorkItem& r = CreatePrivate((void*)func, sizeof(T), memKind);
			if (argPtr)
			{
				void* ap = GetArgPtr(&r);
				*(T*)ap = *argPtr;
			}
			return r;
		}
		static DispatchWorkItem& Create(void (*func)(), MemoryKind memKind)
		{
			static_assert(alignof(DispatchWorkItem) <= sizeof(void*));
			DispatchWorkItem& r = CreatePrivate((void*)func, 0, memKind);
			return r;
		}
		CORE_API void Perform();
		CORE_API void Destroy();
	private:
		CORE_API static DispatchWorkItem& CreatePrivate(void* f, size_t aSize, MemoryKind memKind);
		FORCEINLINE static void* GetArgPtr(DispatchWorkItem* item)
		{
			return ((u8*)item) + sizeof(DispatchWorkItem);
		}
		CORE_API DispatchWorkItem(void* fptr, MemoryKind kind);
	private:
		void*				mFPtr;
		MemoryKind			mMemKind;
	};


	class DispatchGroup
	{
	public:
		CORE_API static DispatchGroup& Create(size_t enterCount);
		CORE_API ~DispatchGroup();
		CORE_API void Destroy();
		CORE_API void Enter();
#if OMNI_DEBUG
		CORE_API void Lock();
#endif
		CORE_API void Leave();
		CORE_API void Notify(DispatchWorkItem& item, LockQueue* queue);
	private:
		CORE_API DispatchGroup(size_t enterCount);
		PrivateData<CPU_CACHE_LINE_SIZE>	mData;
	};
}
