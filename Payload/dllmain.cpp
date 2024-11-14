#include <windows.h>

#define HISTORY_SIZE 1024

DWORD64 exceptionRipHistory[HISTORY_SIZE];
DWORD64 historyIndex = 0;

static LONG WINAPI MyFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	DWORD64 numBytesToSkip = 1; // The instruction that brought us here to begin with
	for (DWORD64 i = 0; i < HISTORY_SIZE; ++i)
	{
		if (exceptionRipHistory[i] == pExceptionInfo->ContextRecord->Rip)
			++numBytesToSkip; // We've already encountered a crash of this instruction just very recently, lets try to skip more in case we are in some kind of exception loop
	}

	exceptionRipHistory[historyIndex] = pExceptionInfo->ContextRecord->Rip;
	if (++historyIndex == HISTORY_SIZE)
		historyIndex = 0;
	
	pExceptionInfo->ContextRecord->Rip += numBytesToSkip;
	return EXCEPTION_CONTINUE_EXECUTION;
}

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID)
{
	// just do it every time, don't care.
	SetUnhandledExceptionFilter(MyFilter);

	return TRUE;
}

