#include <vector>
#include <iostream>
#include <cstring>

#include <framework.h>
#include <ntdll.h>
#include "mem.h"

#pragma comment(lib, "ntdll_x86.lib")

//https://guidedhacking.com/threads/how-to-hack-any-game-first-internal-hack-dll-tutorial.12142/

constexpr unsigned int CALL = 0xE8;
constexpr unsigned int JMP = 0xE9;
constexpr unsigned int NOP = 0x90;
constexpr unsigned int CALL_SIZE = 5;
constexpr unsigned int JMP_SIZE = 5;

bool mem::Detour32(void* src, void* dst, int len)
{
	// Check if len is 5 or greater to make sure the JMP instruction can fit.
	if (len < CALL_SIZE) return false;

	// Setting EXECUTE+READ+WRITE permission for the bytes to alter.
	DWORD curProtection;
	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProtection);

	// Allocating memory for stolen bytes.
	void* gateway = VirtualAlloc(0, len + CALL_SIZE + CALL_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (gateway == nullptr)
	{
		std::cout << "[i] Couldn't allocate memory for stolen bytes" << std::endl;
		return false;
	}

	// Copying stolen bytes to allocated memory.
	std::memcpy(gateway, src, len);

	// Filling the source with NOPs (needed for len > 5).
	std::memset(src, NOP, len);

	// Calculating relative addresses for JMP from source, CALL from gateway and JMP from gateway.
	uintptr_t relativeAddress = ((uintptr_t)gateway - (uintptr_t)src) - CALL_SIZE;
	intptr_t  gatewayToSourceRelativeAddr = (intptr_t)src - (intptr_t)gateway - CALL_SIZE - JMP_SIZE;
	intptr_t  gatewayToDestinationRelativeAddr = (intptr_t)dst - (intptr_t)gateway - CALL_SIZE - JMP_SIZE;

	// Setting the JMP from source up.
	*(BYTE*)src = JMP;
	*(uintptr_t*)((uintptr_t)src + 1) = relativeAddress;

	// Restoring original page protection for source
	DWORD temp;
	VirtualProtect(src, len, curProtection, &temp);

	// Setting the CALL to destination up.
	*(char*)((uintptr_t)gateway + len) = CALL;
	*(uintptr_t*)((uintptr_t)gateway + len + 1) = gatewayToDestinationRelativeAddr;

	// Setting the JMP back from gateway up.
	*(uintptr_t*)((uintptr_t)gateway + len + 5) = JMP;
	*(uintptr_t*)((uintptr_t)gateway + len + 6) = gatewayToSourceRelativeAddr;

	return true;
}

uintptr_t mem::FindDMAAddy(uintptr_t ptr, const std::vector<unsigned int>& offsets)
{
	uintptr_t addr = ptr;
	for (const auto& offset : offsets)
	{
		addr = *(uintptr_t*)addr;
		addr += offset;
	}
	return addr;
}

char* mem::ScanBasic(char* pattern, char* mask, char* begin, intptr_t size)
{
	intptr_t patternLen = strlen(mask);

	for (int i = 0; i < size; i++)
	{
		bool found = true;
		for (int j = 0; j < patternLen; j++)
		{
			if (mask[j] != '?' && pattern[j] != *(char*)((intptr_t)begin + i + j))
			{
				found = false;
				break;
			}
		}
		if (found)
		{
			return (begin + i);
		}
	}
	return nullptr;
}

bool mem::Hook(char* src, char* dst, int len)
{
	if (len < 5) return false;

	DWORD curProtection;

	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProtection);

	memset(src, 0x90, len);

	uintptr_t relativeAddress = (uintptr_t)(dst - src - 5);

	*src = (char)0xE9;
	*(uintptr_t*)(src + 1) = (uintptr_t)relativeAddress;

	DWORD temp;
	VirtualProtect(src, len, curProtection, &temp);

	return true;
}

bool mem::HookFn(char* src, char* dst, int len)
{
	if (len < 5) return false;

	DWORD curProtection;

	VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &curProtection);

	memset(src, 0x90, len);

	uintptr_t relativeAddress = (uintptr_t)(dst - src - 5);

	*src = (char)0xE8;
	*(uintptr_t*)(src + 1) = (uintptr_t)relativeAddress;

	DWORD temp;
	VirtualProtect(src, len, curProtection, &temp);

	return true;
}

char* mem::TrampHook(char* src, char* dst, unsigned int len)
{
	if (len < 5) return 0;
	char* gateway = (char*)VirtualAlloc(0, len + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	memcpy(gateway, src, len);

	uintptr_t gateJmpAddy = (uintptr_t)(src - gateway - 5);
	*(gateway + len) = (char)0xE9;
	*(uintptr_t*)(gateway + len + 1) = gateJmpAddy;

	if (Hook(src, dst, len))
	{
		return gateway;
	}
	else return nullptr;
}

char* mem::ScanInternal(char* pattern, char* mask, char* begin, intptr_t size)
{
	char* match{ nullptr };
	MEMORY_BASIC_INFORMATION mbi{};

	for (char* curr = begin; curr < begin + size; curr += mbi.RegionSize)
	{
		if (!VirtualQuery(curr, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS) continue;

		match = ScanBasic(pattern, mask, curr, mbi.RegionSize);

		if (match != nullptr)
		{
			break;
		}
	}
	return match;
}

void mem::WriteMemory(LPVOID address, LPVOID value, int byteNum)
{
	unsigned long OldProtection;
	VirtualProtect(address, byteNum, PAGE_EXECUTE_READWRITE, &OldProtection);
	memcpy(address, value, byteNum);
	VirtualProtect(address, byteNum, OldProtection, &OldProtection);
}

char* TO_CHAR(wchar_t* string)
{
	size_t len = wcslen(string) + 1;
	char* c_string = new char[len];
	size_t numCharsRead;
	wcstombs_s(&numCharsRead, c_string, len, string, _TRUNCATE);
	return c_string;
}

PEB* GetPEB()
{
#ifdef _WIN64
	PEB* peb = (PEB*)__readgsqword(0x60);

#else
	PEB* peb = (PEB*)__readfsdword(0x30);
#endif
	return peb;
}

LDR_DATA_TABLE_ENTRY* GetLDREntry(std::string name, bool forceSystem)
{
	LDR_DATA_TABLE_ENTRY* ldr = nullptr;

	PEB* peb = GetPEB();

	LIST_ENTRY head = peb->Ldr->InMemoryOrderModuleList;

	LIST_ENTRY curr = head;

	WCHAR system_dir[MAX_PATH];
	GetSystemDirectoryW(system_dir, MAX_PATH);

	while (curr.Flink != head.Blink)
	{
		LDR_DATA_TABLE_ENTRY* mod = (LDR_DATA_TABLE_ENTRY*)CONTAINING_RECORD(curr.Flink, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

		if (mod->FullDllName.Buffer)
		{
			char* cName = TO_CHAR(mod->BaseDllName.Buffer);

			if (_stricmp(cName, name.c_str()) == 0)
			{
				if (!forceSystem || (forceSystem && std::wstring(mod->FullDllName.Buffer).rfind(system_dir, 0) == 0)) {
					ldr = mod;
					break;
				}
			}
			delete[] cName;
		}
		curr = *curr.Flink;
	}
	return ldr;
}

char* mem::ScanModIn(char* pattern, char* mask, std::string modName, bool forceSystem)
{
	LDR_DATA_TABLE_ENTRY* ldr = GetLDREntry(modName, forceSystem);
	std::cout << "[i] DLL base for " << modName << ": " << ldr->DllBase << std::endl;
	std::cout << "[i] Size of image: " << ldr->SizeOfImage << std::endl;
	char* match = mem::ScanInternal(pattern, mask, (char*)ldr->DllBase, ldr->SizeOfImage);
	return match;
}

