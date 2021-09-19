#pragma once
#include "Runtime/Omni.h"

namespace Omni
{
	class ThreadLocalDataImpl
	{
	public:
		ThreadLocalDataImpl();
		FORCEINLINE void SetImpl(void* v);
		FORCEINLINE void* GetImpl() const;
	private:
		u64 mKey;
	};

	template<typename T>
	class ThreadLocalData : private ThreadLocalDataImpl
	{
		static_assert(sizeof(T) <= sizeof(void*), "sizeof(T) <= sizeof(void*)");
	public:
		FORCEINLINE void Set(T v) { SetImpl(reinterpret_cast<void*>(v)); }
		FORCEINLINE T Get() const { return reinterpret_cast<T>(GetImpl()); }
	};
}

