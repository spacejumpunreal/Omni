#include "Runtime/Base/BasePCH.h"
#include "Runtime/Base/MultiThread/IThreadLocal.h"
#include "Runtime/Base/Misc/AssertUtils.h"

namespace Omni
{
    static thread_local IThreadLocal* gIThreadLocalHead; //the only raw thread_local that is allowed
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

