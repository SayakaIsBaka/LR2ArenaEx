#include <utils/mem.h>
#include <client/client.h>
#include <network/enums.h>

#include "returnmenu.h"

void hkReturnMenu() {
	fprintf(stdout, "returning to menu\n");
	hooks::return_menu::is_returning_to_menu = true;
	client::Send(network::ClientToServer::CTS_CHART_CANCELLED, ""); // send escaped
}

DWORD jmp_lr2body_419863 = 0x419863;
__declspec(naked) unsigned int trampReturnMenu() {

	__asm {
		// hook [esi+97BA9h] = return menu
		pushfd
		pushad
		call hkReturnMenu
		popad
		popfd
		// end hook
		mov byte ptr[esi + 97BA9h], 1
		add esp, 4
		jmp jmp_lr2body_419863
	}
}

void hooks::return_menu::Setup() {
	mem::HookFn((char*)0x41985C, (char*)trampReturnMenu, 7);
}