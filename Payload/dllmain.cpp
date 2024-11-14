#include <windows.h>

#define HISTORY_SIZE 1024

DWORD64 exceptionRipHistory[HISTORY_SIZE];
DWORD64 historyIndex = 0;

LONG WINAPI MyFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	DWORD64 instructionsToSkip = 1; // The instruction that brought us here to begin with
	for (DWORD64 i = 0; i < sizeof(HISTORY_SIZE); ++i)
	{
		if (exceptionRipHistory[i] == pExceptionInfo->ContextRecord->Rip)
		{
			instructionsToSkip++; // We've already encountered a crash of this instruction just very recently, lets try to skip more in case we are in some kind of exception loop
			break;
		}
	}

	exceptionRipHistory[historyIndex] = pExceptionInfo->ContextRecord->Rip;
	historyIndex = (historyIndex + 1) / HISTORY_SIZE;
	
    pExceptionInfo->ContextRecord->Rip += instructionsToSkip;
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

