#include "Runtime/Base/BasePCH.h"
#include "Runtime/Base/MultiThread/ThreadLocalData.h"
#include "Runtime/Base/Misc/AssertUtils.h"

#if OMNI_WINDOWS
#include <windows.h> 
#endif

namespace Omni
{
#if OMNI_WINDOWS
	ThreadLocalDataImpl::ThreadLocalDataImpl()
		: mKey(TlsAlloc())
	{
		CheckAlways(((DWORD)mKey) != TLS_OUT_OF_INDEXES);
	}
	void ThreadLocalDataImpl::SetImpl(void* v)
	{
		TlsSetValue((DWORD)mKey, v);
	}
	void* ThreadLocalDataImpl::GetImpl() const
	{
		return TlsGetValue((DWORD)mKey);
	}
#else
#endif
}

