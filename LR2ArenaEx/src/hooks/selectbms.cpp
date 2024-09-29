#include <iostream>
#include <utils/mem.h>
#include <client/client.h>

#include "selectbms.h"
#include "pacemaker.h"
#include "random.h"
#include "returnmenu.h"

void hkSelectBms(const char** buffer, unsigned char* memory) {
	unsigned int selected_option = (unsigned int)*(memory + 0x10);
	std::string new_selected_bms = std::string(*buffer);
	if (!new_selected_bms.rfind("LR2files\\Config\\sample_", 0)) {
		fprintf(stdout, "demo BMS loaded, skip\n");
		return;
	}
	hooks::select_bms::selectedBms = new_selected_bms;
	hooks::pacemaker::p2_score = 0; // it's most likely when you start a song so reset score

	std::cout << "[+] Selected BMS: " << hooks::select_bms::selectedBms << std::endl;
	std::cout << "[+] Selected option: " << selected_option << std::endl;

	if (!hooks::random::received_random) {
		hooks::random::UpdateRandom();
	}

	client::SendWithRandom(1, hooks::select_bms::selectedBms);

	hooks::return_menu::is_returning_to_menu = false;
}

DWORD fsopen_addr = 0x715DA0;
__declspec(naked) void trampSelectBms() {
	__asm {
		// hook ecx = ptr ptr bms path
		pushfd
		pushad
		push[esp + 0x4c] // selected option is at [[esp+0x4c] + 0x10]
		push ecx
		call hkSelectBms
		add esp, 8
		popad
		popfd
		// end hook
		mov edi, edi
		push ebp
		mov ebp, esp
		push 0x40
		push dword ptr ss : [ebp + 0xC]
		push dword ptr ss : [ebp + 0x8]
		call fsopen_addr // 00715E71 | E8 2AFFFFFF | call lr2body.715DA0 |
		add esp, 0xC
		pop ebp
		ret
	}
}

void hooks::select_bms::Setup() {
	mem::HookFn((char*)0x4B0D92, (char*)trampSelectBms, 5);
}