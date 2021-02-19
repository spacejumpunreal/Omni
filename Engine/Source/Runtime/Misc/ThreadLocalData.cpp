#include "Runtime/Misc/ThreadLocalData.h"
#include "Runtime/Test/AssertUtils.h"

#if OMNI_WINDOWS
#include <windows.h> 
#else
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

