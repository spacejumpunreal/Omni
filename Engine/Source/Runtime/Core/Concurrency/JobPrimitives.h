#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Base/Container/LinkedList.h"
#include "Runtime/Base/Memory/MemoryDefs.h"
#include "Runtime/Base/Misc/PrivateData.h"
#include "Runtime/Base/MultiThread/LockQueue.h"
#include "Runtime/Prelude/PlatformDefs.h"



namespace Omni
{
	class DispatchWorkItem : public SListNode
	{
	public:
		template<typename Functor>
		static DispatchWorkItem& CreateWithFunctor(Functor&& functor, MemoryKind memKind, bool autoRelease)
		{
			static_assert(std::is_standard_layout_v<Functor> && std::is_trivial_v<Functor>);
			static_assert(alignof(DispatchWorkItem) <= sizeof(void*));
			static_assert(sizeof(decltype(&Functor::Run)) > 0);
			DispatchWorkItem& r = CreatePrivate((void*)Functor::Run, sizeof(Functor), memKind, autoRelease);
			void* ap = GetArgPtr(&r);
			*(Functor*)ap = std::move(functor);
			return r;
		}

		template<typename T>
		static DispatchWorkItem& Create(void (*func)(T*), const T* argPtr, MemoryKind memKind, bool autoRelease)
		{
			static_assert(std::is_standard_layout_v<T> && std::is_trivial_v<T>);
			static_assert(alignof(DispatchWorkItem) <= sizeof(void*));
			DispatchWorkItem& r = CreatePrivate((void*)func, sizeof(T), memKind, autoRelease);
			if (argPtr)
			{
				void* ap = GetArgPtr(&r);
				*(T*)ap = *argPtr;
			}
			return r;
		}
		static DispatchWorkItem& Create(void (*func)(), MemoryKind memKind, bool autoRelease)
		{
			static_assert(alignof(DispatchWorkItem) <= sizeof(void*));
			DispatchWorkItem& r = CreatePrivate((void*)func, 0, memKind, autoRelease);
			return r;
		}
		CORE_API void Perform();
		CORE_API void Release(bool isAutoRelease);
		CORE_API bool IsAutoRelease() { return mSize == 0; }
	private:
		CORE_API static DispatchWorkItem& CreatePrivate(void* f, size_t aSize, MemoryKind memKind, bool autoRelease);
		FORCEINLINE static void* GetArgPtr(DispatchWorkItem* item)
		{
			return ((u8*)item) + sizeof(DispatchWorkItem);
		}
		CORE_API DispatchWorkItem(void* fptr, MemoryKind kind, u32 size);
		CORE_API ~DispatchWorkItem()
		{
			mFPtr = nullptr;
			mMemKind = MemoryKind::Max;
			mSize = (u32) - 1;
		}
	private:
		void*				mFPtr;
		MemoryKind			mMemKind;
		u32					mSize;
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
