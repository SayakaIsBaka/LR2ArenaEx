#pragma once
#include <framework.h>
#include <vector>
#include <string>

//https://guidedhacking.com/threads/how-to-hack-any-game-first-internal-hack-dll-tutorial.12142/

namespace mem
{
	bool Detour32(void* src, void* dst, int len);
	uintptr_t FindDMAAddy(uintptr_t ptr, const std::vector<unsigned int>& offsets);
	bool Hook(char* src, char* dst, int len);
	bool HookFn(char* src, char* dst, int len);
	char* TrampHook(char* src, char* dst, unsigned int len);
	char* ScanBasic(char* pattern, char* mask, char* begin, intptr_t size);
	char* ScanInternal(char* pattern, char* mask, char* begin, intptr_t size);
	char* ScanModIn(char* pattern, char* mask, std::string modName);
}