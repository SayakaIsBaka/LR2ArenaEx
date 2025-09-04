#include <fstream>
#include <iostream>
#include <utils/mem.h>
#include <hooks/hooks.h>
#include <config/config.h>
#include <ImGui/ImGuiNotify.hpp>
#include <MinHook.h>

#include "overlay.h"
#include "dx9hook.h"
#include "dinputhook.h"

#ifndef _DEBUG
#define DEBUG_CONSOLE_ENABLED 0
#else
#define DEBUG_CONSOLE_ENABLED 1
#endif

DWORD WINAPI overlay::Setup(HMODULE hModule)
{
#if DEBUG_CONSOLE_ENABLED
	AllocConsole();
	FILE* f = nullptr;
	freopen_s(&f, "CONOUT$", "w", stdout);
	SetConsoleOutputCP(65001); // Set code page to UTF-8

	std::cout << "--- LR2ArenaEx debugging console ---" << std::endl << std::endl;
#endif
	moduleBase = (uintptr_t)GetModuleHandle(NULL);
	lr2res.x = *(unsigned int*)0x4307AD;
	lr2res.y = *(unsigned int*)0x4307A8;
	lr2type = lr2res.y >= 720 ? LR2_TYPE::LR2_HD : LR2_TYPE::LR2_SD;

	MH_Initialize();

	dx9hook::HookDX9();
	dinputhook::HookDinput(hModule);
	hooks::SetupHooks();
	config::LoadConfig();

#if DEBUG_CONSOLE_ENABLED
	while (true)
	{
	}
	fclose(f);
	FreeConsole();
#endif

#pragma warning(push)
#pragma warning(disable : 6258)
	TerminateThread(GetCurrentThread(), 0);
#pragma warning(pop)
	return 0;
}