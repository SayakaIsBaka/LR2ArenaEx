#include <utils/mem.h>
#include <client/client.h>
#include <network/enums.h>
#include <iostream>
#include <gui/graph.h>

#include "returnmenu.h"
#include "maxscore.h"

void hkReturnMenu() {
	std::cout << "[+] Returning to menu" << std::endl;
	auto score = client::state.peers[client::state.remoteId].score;
	// Do not send cancelled if pressing escape at the end of the chart (to show results faster)
	if (((score.p_great + score.great + score.good + score.bad + score.poor) * 2 >= hooks::max_score::maxScore && client::state.peers[client::state.remoteId].ready)) {
		return;
	}
	hooks::return_menu::is_returning_to_menu = true;
	gui::graph::showGraph = false;
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