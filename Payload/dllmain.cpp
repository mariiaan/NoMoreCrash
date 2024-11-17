#include <windows.h>
#include <csignal>
#include <exception>
#include <cstring>
#include <stdio.h>
#include <vector>

#define TRY_DEFEAT_LOOPS 1

#define HISTORY_SIZE 1024

#if TRY_DEFEAT_LOOPS
DWORD64 exceptionRipHistory[HISTORY_SIZE];
DWORD64 historyIndex = 0;
#endif

#define OP_NOOP 0x90
#define OP_RET 0xC3
#define OP_JNE 0x75
#define OP_JE 0x74
#define OP_JMP 0xEB

static void DestroyLoops(void* currentRip, int range = 64)
{
	std::vector<unsigned char> instructionsToPatch = { OP_JNE, OP_JE, OP_JMP };
	auto p = (unsigned char*)currentRip;
	for (auto pi = p - range; pi < p + range; ++pi)
	{
		DWORD oldProtect;
		VirtualProtect(pi, 1, PAGE_READWRITE, &oldProtect);

		auto it = std::find(instructionsToPatch.begin(), instructionsToPatch.end(), *pi);
		if (it != instructionsToPatch.end())
			*pi = OP_NOOP; // NOP
		VirtualProtect(pi, 1, oldProtect, &oldProtect);
	}
}

static LONG WINAPI MyFilter(PEXCEPTION_POINTERS pExceptionInfo)
{	
#if TRY_DEFEAT_LOOPS
	for (DWORD64 i = 0; i < HISTORY_SIZE; ++i)
	{
		if (exceptionRipHistory[i] == pExceptionInfo->ContextRecord->Rip)
		{
			DestroyLoops((void*)pExceptionInfo->ContextRecord->Rip);
			break;
		}
	}

	exceptionRipHistory[historyIndex] = pExceptionInfo->ContextRecord->Rip;
	historyIndex = (historyIndex + 1) % HISTORY_SIZE;
#endif
	pExceptionInfo->ContextRecord->Rip++;
	return EXCEPTION_CONTINUE_EXECUTION;
}

static void MakeFunctionReturn(void* pFunction)
{
	DWORD oldProtect;
	VirtualProtect(pFunction, 1, PAGE_READWRITE, &oldProtect);
	*(unsigned char*)pFunction = OP_RET;
	VirtualProtect(pFunction, 1, oldProtect, &oldProtect);
}

BOOL APIENTRY DllMain(HMODULE, DWORD ulReason, LPVOID)
{
#if TRY_DEFEAT_LOOPS
	if (ulReason == DLL_PROCESS_ATTACH)
		memset(exceptionRipHistory, 0, HISTORY_SIZE);
#endif

	// just do it every time, don't care.
	SetUnhandledExceptionFilter(MyFilter);
	
	MakeFunctionReturn(abort);
	MakeFunctionReturn(ExitProcess);
	MakeFunctionReturn(terminate);

	return TRUE;
}

