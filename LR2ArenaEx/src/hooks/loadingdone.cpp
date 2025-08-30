#include <utils/mem.h>
#include <client/client.h>
#include <network/enums.h>
#include <ImGui/ImGuiNotify.hpp>

#include "loadingdone.h"
#include "random.h"
#include "returnmenu.h"

void hkLoadingDone() {
	if (client::connected) {
		if (!hooks::return_menu::is_returning_to_menu) {
			client::Send(network::ClientToServer::CTS_LOADING_COMPLETE, ""); // player ready, send packet
			ImGui::InsertNotification({ ImGuiToastType::Info, 3000, "Loading complete, waiting for all players..." });
			fprintf(stdout, "waiting for all players\n");
			while (!hooks::loading_done::isEveryoneReady)
			{
				SleepEx(50, false);
			}
			hooks::loading_done::isEveryoneReady = false;
			hooks::random::received_random = false;
			fprintf(stdout, "lock released\n");
		}
		else {
			hooks::return_menu::is_returning_to_menu = false; // might not be needed because of reset in select_bms but keeping it just in case
		}
	}
}

DWORD jmp_lr2body_42CC4A = 0x42CC4A;
int(__cdecl* SetTimeLapse)(int, void*) = (int(__cdecl*)(int, void*))0x4B6B80;
__declspec(naked) unsigned int trampLoadingDone() {

	__asm {
		pushfd
		pushad
		call hkLoadingDone
		popad
		popfd
		// end hook
		call SetTimeLapse
		jmp jmp_lr2body_42CC4A
	}
}

void hooks::loading_done::Setup() {
	mem::Hook((char*)0x42CC45, (char*)trampLoadingDone, 5);
}