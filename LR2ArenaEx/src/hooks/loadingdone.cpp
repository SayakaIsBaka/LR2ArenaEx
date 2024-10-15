#include <utils/mem.h>
#include <client/client.h>
#include <network/enums.h>

#include "loadingdone.h"
#include "random.h"
#include "returnmenu.h"

void hkLoadingDone() {
	if (!hooks::return_menu::is_returning_to_menu) {
		client::Send(network::ClientToServer::CTS_LOADING_COMPLETE, ""); // player ready, send packet
		fprintf(stdout, "waiting for p2\n");
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

DWORD jmp_lr2body_40848D = 0x40848D;
__declspec(naked) unsigned int trampLoadingDone() {

	__asm {
		// hook [esi+97BAAh] = loading done
		pushfd
		pushad
		call hkLoadingDone
		popad
		popfd
		// end hook
		mov byte ptr[esi + 97BAAh], 1
		add esp, 4
		jmp jmp_lr2body_40848D
	}
}

void hooks::loading_done::Setup() {
	mem::HookFn((char*)0x408486, (char*)trampLoadingDone, 7);
}