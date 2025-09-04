#include <utils/mem.h>
#include <gui/graph.h>
#include <iostream>
#include <sqlite3.h>
#include <MinHook.h>
#include <ImGui/ImGuiNotify.hpp>

#include "cstr.h"
#include "selectscene.h"

void hkSelectScene() {
	static bool showBindNotif = true;
	if (showBindNotif) {
		ImGui::InsertNotification({ ImGuiToastType::Success, 3000, "LR2ArenaEx successfully loaded; press %s to show the overlay!",
		utils::keys::toString(utils::keys::bindings[utils::keys::BindingType::MENU_TOGGLE].key).c_str() });
		showBindNotif = false;
	}

	if (gui::graph::automaticGraph) {
		gui::graph::showGraph = false;
	}
}

DWORD jmp_lr2body_43C510 = 0x43C510;
__declspec(naked) unsigned int trampSelectScene() {

	__asm {
		pushfd
		pushad
		call hkSelectScene
		popad
		popfd
		// end hook
		jmp jmp_lr2body_43C510
	}
}

int __cdecl hkProcSelect(void* game, void* sql) {
	if (hooks::select_scene::game == NULL)
		hooks::select_scene::game = game;
	return hooks::select_scene::oProcSelect(game, sql);
}

void hooks::select_scene::SearchSongByHash(std::string hash) {
	if (game == NULL) {
		std::cerr << "[!] Game structure couldn't be retrieved" << std::endl; // This shouldn't happen
		return;
	}
	char* query = 0;
	cstr::CstrInit(&query);
	char q[1024] = { 0 };
	sqlite3_snprintf(1024, q, "SELECT * FROM song LEFT JOIN score ON song.hash = score.hash WHERE song.hash = '%q'", hash.c_str());
	cstr::CstrAssign(&query, q);

	char* searchInput = 0;
	cstr::CstrInit(&searchInput);
	cstr::CstrAssign(&searchInput, hash.c_str());

	int* dwGame = (int*)game;
	dwGame[84] = 0;
	dwGame[7034] = 1;
	dwGame[7035] = 0;
	dwGame[7036] = dwGame[86];
	cstr::CstrCopy((char**)(dwGame + 7022), &query);
	dwGame[7020] = 1;
	dwGame[7021] = 3;
	dwGame[7038] = dwGame[87];
	dwGame[7037] = dwGame[85];
	dwGame[7030] = 0;
	cstr::CstrCopy((char**)(dwGame + 7041), (char**)(dwGame[1929] + 712 * dwGame[1945]));
	cstr::CstrCopy((char**)(dwGame + 7040), (char**)(dwGame[1929] + 712 * dwGame[1945] + 20));
	cstr::CstrCopy((char**)(dwGame + 7042), (char**)(dwGame[1929] + 712 * dwGame[1945] + 36));
	cstr::CstrCopy((char**)(dwGame + 7044), &searchInput);
	dwGame[7045] = 30; // Value used when the search bar is selected

	cstr::CstrFree(&query);
	cstr::CstrFree(&searchInput);
}

void hooks::select_scene::Setup() {
	mem::HookFn((char*)0x432006, (char*)trampSelectScene, 5);

	oProcSelect = (ProcSelect)0x4281A0;
	if (MH_CreateHookEx((LPVOID)oProcSelect, &hkProcSelect, &oProcSelect) != MH_OK)
	{
		std::cout << "[!] Error hooking ProcSelect" << std::endl;
		return;
	}

	if (MH_QueueEnableHook(MH_ALL_HOOKS) || MH_ApplyQueued() != MH_OK)
	{
		std::cout << "[!] Error enabling ProcSelect hook" << std::endl;
		return;
	}
}