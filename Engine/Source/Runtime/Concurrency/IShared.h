#pragma once
#include "Runtime/Omni.h"
#include <atomic>

namespace Omni
{
	class IShared
	{
	public:
		IShared() : mRefCount(1) 
		{}
		void Retain() 
		{
			mRefCount.fetch_add(1, std::memory_order::relaxed);
		}
		void Release()
		{
			u32 o = mRefCount.fetch_sub(1, std::memory_order::acq_rel);
			if (o == 1)
				Destroy();
		}
	protected:
		virtual void Destroy() = 0; //we have custom allocation, can't just call delete, dtor should be called in this
	private:
		std::atomic<u32>	mRefCount;
	};
	template<typename T>
	class SharedPtr
	{
	public:
		static_assert(std::is_base_of_v<IShared, T>, "T must be derived from IShared");
	public:
		SharedPtr(T* p = nullptr)
			: mRawPtr(p)
		{
			if (p)
				p->Retain();
		}
		~SharedPtr()
		{
			if (mRawPtr)
				mRawPtr->Release();
		}
		SharedPtr& operator=(T* p)
		{
			p->Retain();
			if (mRawPtr)
				mRawPtr->Release();
			mRawPtr = p;
		}
		T* operator->() const
		{
			return mRawPtr;
		}
		operator bool() const
		{
			return mRawPtr != nullptr;
		}
		T* Raw() const
		{
			return mRawPtr;
		}
	private:
		T*	mRawPtr;
	};
}