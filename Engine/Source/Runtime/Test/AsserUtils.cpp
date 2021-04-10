#include "Runtime/Omni.h"
#include "Runtime/Test/AssertUtils.h"
#include <Windows.h>
namespace Omni
{
    void BreakAndCrash(volatile int* p)
    {
        *p = 0;
        if (p != nullptr)
            BreakAndCrash(nullptr);
    }

#if OMNI_WINDOWS
    void CheckWinAPI(int x)
    {
        if (x == 0)
        {
            wchar_t* lpMsgBuf;
            DWORD dw = GetLastError();

            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                dw,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpMsgBuf,
                0, NULL);
            
            DebugBreak();
            LocalFree(lpMsgBuf);
        }
    }
    void OmniDebugBreak()
    {
        DebugBreak();
    }
#endif
}