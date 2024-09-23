#include "Injector.h"
#include <iostream>
#include <fstream>
#include <Windows.h>
#include <TlHelp32.h>
#include <filesystem>

// Iterate over the list of process names until procName is found.
static DWORD GetProcId(const char* procName)
{
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry) != 0)
		{
			do
			{
				if (strcmp(procEntry.szExeFile, procName) == 0)
				{
					procId = procEntry.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &procEntry));
		}
	}
	CloseHandle(hSnap);
	return procId;
}

static std::wstring GetExecutableDir()
{
	WCHAR dllPath[MAX_PATH];
	GetModuleFileNameW(NULL, dllPath, MAX_PATH);
	std::filesystem::path wPath(dllPath);
	wPath.remove_filename();
	return wPath.native();
}

int Injector()
{
	constexpr const WCHAR* dllName = L"D3D9Hook.dll";
	constexpr const char* procName = "LRHbody.exe";
	std::wstring dllPath = GetExecutableDir().append(dllName);
	DWORD procId = 0;

	procId = GetProcId(procName);
	if (procId == 0)
	{
		procId = GetProcId("LR2body.exe");
		if (procId == 0)
		{
			std::cout << "Couldn't find process\n";
			return 1;
		}
	}

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procId);

	if (hProc == nullptr || hProc == INVALID_HANDLE_VALUE)
	{
		std::cout << "Process is found, but couldn't get a handle to it\n";
		return 1;
	}

	void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (loc == nullptr)
	{
		std::cout << "Couldn't allocate memory in the remote process\n";
		return 1;
	}

	if (WriteProcessMemory(hProc, loc, dllPath.c_str(), (dllPath.size() + 1) * sizeof(WCHAR), 0) == 0)
	{
		std::cout << "Couldn't write .dll to memory\n";
		return 1;
	}


	HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryW, loc, 0, 0);

	if (hThread != nullptr)
	{
		CloseHandle(hThread);
	}

	else
	{
		std::cout << "Couldn't start remote thread of the .dll\n";
		return 1;
	}

	CloseHandle(hProc);

	return 0;
}