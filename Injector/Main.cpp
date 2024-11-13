#include <Windows.h>
#include <tlhelp32.h>
#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

// This program injects a DLL that overrides the ExceptionFilter function with a function that just increments the RIP register
// and let the program continue execution. This way, the program won't crash anymore.
// Things like std::abort() will still crash the program because they are not exceptions.

#if NDEBUG  // release
	#define PAYLOAD_SIZE 10240
#else		// debug
	#define PAYLOAD_SIZE 59904
#endif

static bool InjectDLL(uint32_t pid, const std::string& dllName)
{
	std::cout << "Injecting DLL into process with PID: " << pid << std::endl;

	char dllPath[MAX_PATH];
	GetFullPathNameA("Payload.dll", MAX_PATH, dllPath, NULL);
	std::cout << "Full DLL path is: " << dllPath << std::endl << std::endl;

	std::cout << "Opening process..\t\t\t";
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess == NULL)
	{
		std::cout << "FAILED" << std::endl;
		return false;
	}
	std::cout << "SUCCESS" << std::endl;

	std::cout << "Allocating memory in remote process..\t";
	LPVOID pDllPath = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (pDllPath == NULL)
	{
		std::cout << "FAILED" << std::endl;
		return false;
	}
	std::cout << "SUCCESS" << std::endl;

	std::cout << "Writing memory in remote process..\t";

	if (!WriteProcessMemory(hProcess, pDllPath, dllPath, MAX_PATH, NULL))
	{
		std::cout << "FAILED" << std::endl;
		return false;
	}
	std::cout << "SUCCESS" << std::endl;

	std::cout << "Creating remote thread..\t\t";
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, pDllPath, 0, NULL);
	if (hThread == NULL)
	{
		std::cout << "FAILED" << std::endl;
		return false;
	}
	std::cout << "SUCCESS" << std::endl;

	CloseHandle(hThread);
	CloseHandle(hProcess);
	std::cout << "Successfully injected DLL :)" << std::endl;

	return true;
}

static bool ExtractFromOwnFile(size_t size, const std::string& outFile)
{
	std::cout << "Extracting resource \"" << outFile << "\" with size " << size << std::endl;

	std::ifstream ifs1(outFile, std::ios::binary | std::ios::ate);
	if (ifs1.is_open())
		return true;
	ifs1.close();

	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);

	std::ifstream ifs(path, std::ios::binary | std::ios::ate);
	if (!ifs.is_open())
	{
		std::cout << "Failed to extract resources" << std::endl;
		return false;
	}

	size_t ownFileSize = ifs.tellg();
	size_t sizeWithoutPayload = ownFileSize - size;
	ifs.seekg(sizeWithoutPayload, std::ios::beg);

	std::vector<char> payload(size);
	ifs.read(payload.data(), size);
	ifs.close();

	std::ofstream ofs(outFile, std::ios::binary);
	if (!ofs.is_open())
	{
		std::cout << "Failed to extract resources" << std::endl;
		return false;
	}
	ofs.write(payload.data(), size);
	ofs.close();

	std::cout << "Successfully extracted resources" << std::endl;

	return true;
}

static DWORD GetPid(const std::wstring& imageName)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		std::cout << "Failed to create snapshot" << std::endl;
		return 0;
	}

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnapshot, &pe32))
	{
		std::cout << "Failed to get first process" << std::endl;
		CloseHandle(hSnapshot);
		return 0;
	}

	do
	{
		if (imageName == std::wstring(pe32.szExeFile))
		{
			CloseHandle(hSnapshot);
			return pe32.th32ProcessID;
		}
	} while (Process32Next(hSnapshot, &pe32));

	CloseHandle(hSnapshot);
	return 0;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		std::cerr << "Usage: NoMoreCrash.exe <PID> OR NoMoreCrash.exe --im <NAME>" << std::endl;
		return 1;
	}

	if (!ExtractFromOwnFile(PAYLOAD_SIZE, "Payload.dll"))
		return 1;

	std::vector<std::string> args(argv + 1, argv + argc);
	DWORD targetPid = 0;

	if (args[0] == "--im")
	{
		if (args.size() < 2)
		{
			std::cerr << "Usage: NoMoreCrash.exe --im <NAME>" << std::endl;
			return 1;
		}

		std::wstring targetImageName(args[1].begin(), args[1].end());
		targetPid = GetPid(targetImageName);
		if (targetPid == 0)
		{
			std::cerr << "Failed to get target PID" << std::endl;
			return 1;
		}
	}
	else
	{
		targetPid = std::stoi(args[0]);
	}

	if (!InjectDLL(targetPid, "Payload.dll"))
	{
		std::cerr << "Failed to inject DLL" << std::endl;
		return 1;
	}

	std::cout << "Enjoy no more crashes!" << std::endl;

	return 0;
}