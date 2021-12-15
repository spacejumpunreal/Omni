#pragma once
#include "Runtime/Prelude/Omni.h"

#define RELEASE_COM(comPtr) { comPtr->Release(); comPtr = nullptr; }

namespace Omni
{
	template<typename ComT>
	FORCEINLINE void SafeRelease(ComT*& com)
	{
		com->Release();
		com = nullptr;
	}

	template<typename ComT>
	class ComPtr
	{
	public:
		FORCEINLINE ComPtr(ComT*&& ptr = nullptr) 
			: mPtr(ptr)
		{//transfer ownership
			ptr = nullptr;
		}
		FORCEINLINE ComPtr(const ComPtr& ptr)
			: mPtr(ptr.mPtr)
		{//ref+1
			if (mPtr)
				mPtr->AddRef();
		}
		FORCEINLINE ~ComPtr()
		{//release old ref
			if (mPtr)
				mPtr->Release();
			mPtr = nullptr;
		}
		FORCEINLINE ComPtr& operator=(const ComT& ptr)
		{//release old ref,new ref+1
			if (mPtr)
				mPtr->Release();
			mPtr = ptr;
			if (mPtr)
				mPtr->AddRef();
			return *this;
		}
		FORCEINLINE ComPtr& operator=(ComT*&& ptr)
		{//release ownership
			if (mPtr)
				mPtr->Release();
			mPtr = ptr;
			ptr = nullptr;
			return *this;
		}
		FORCEINLINE ComT* operator->() { return mPtr; }
		FORCEINLINE ComT** operator&() { return &mPtr; }
		FORCEINLINE operator ComT* () { return mPtr; }
	private:
		ComT* mPtr;
	};
}