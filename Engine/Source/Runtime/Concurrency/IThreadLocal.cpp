#include "Runtime/Concurrency/IThreadLocal.h"
#include "Runtime/Test/AssertUtils.h"

namespace Omni
{
    static thread_local IThreadLocal* gIThreadLocalHead; //only raw thread_local that is allowed
    IThreadLocal::IThreadLocal()
        : mNext(gIThreadLocalHead)
    {
        gIThreadLocalHead = mNext;
    }
    void IThreadLocal::CheckAllThreadLocalClean()
    {
        IThreadLocal* p = gIThreadLocalHead;
        while (p)
        {
            CheckAlways(p->IsClean());
            p = p->mNext;
        }
    }
    IThreadLocal* IThreadLocal::GetAllThreadLocals()
    {
        return gIThreadLocalHead;
    }
    void IThreadLocal::CheckIsClean()
    {
        CheckAlways(IsClean());
     }
}