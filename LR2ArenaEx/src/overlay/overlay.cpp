#include <fstream>
#include <iostream>
#include <utils/mem.h>
#include <hooks/hooks.h>
#include <ImGui/ImGuiNotify.hpp>

#include "overlay.h"
#include "dx9hook.h"
#include "dinputhook.h"

#define DEBUG_CONSOLE_ENABLED

DWORD WINAPI overlay::Setup(HMODULE hModule)
{
#ifdef DEBUG_CONSOLE_ENABLED
	AllocConsole();
	FILE* f = nullptr;
	freopen_s(&f, "CONOUT$", "w", stdout);
	SetConsoleOutputCP(65001); // Set code page to UTF-8

	std::cout << "--- LR2ArenaEx debugging console ---" << std::endl << std::endl;
#endif
	if ((moduleBase = (uintptr_t)GetModuleHandle("LRHbody.exe")) == 0)
	{
		moduleBase = (uintptr_t)GetModuleHandle("LR2body.exe");
		lr2type = LR2_TYPE::LR2_SD;
	}

	dx9hook::HookDX9();
	dinputhook::HookDinput(hModule);
	hooks::SetupHooks();

	ImGui::InsertNotification({ ImGuiToastType::Success, 3000, "LR2ArenaEx successfully loaded; press [Ins] to show the overlay!" });

#ifdef DEBUG_CONSOLE_ENABLED
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