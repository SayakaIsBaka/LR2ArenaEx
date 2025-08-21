#include <fstream>
#include <iostream>
#include <utils/mem.h>
#include <hooks/hooks.h>
#include <config/config.h>
#include <ImGui/ImGuiNotify.hpp>

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
	if ((moduleBase = (uintptr_t)GetModuleHandle("LRHbody.exe")) == 0)
	{
		moduleBase = (uintptr_t)GetModuleHandle("LR2body.exe");
		lr2type = LR2_TYPE::LR2_SD;
	}

	dx9hook::HookDX9();
	dinputhook::HookDinput(hModule);
	hooks::SetupHooks();
	config::LoadConfig();

	ImGui::InsertNotification({ ImGuiToastType::Success, 3000, "LR2ArenaEx successfully loaded; press %s to show the overlay!",
		utils::keys::toString(utils::keys::bindings[utils::keys::BindingType::MENU_TOGGLE].key).c_str() });

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