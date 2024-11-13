#include <windows.h>

LONG WINAPI MyFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
    pExceptionInfo->ContextRecord->Rip++; 

    return EXCEPTION_CONTINUE_EXECUTION;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    // just do it every time, don't care.
	SetUnhandledExceptionFilter(MyFilter);

    return TRUE;
}

