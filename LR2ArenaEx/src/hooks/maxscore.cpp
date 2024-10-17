#include <utils/mem.h>
#include <iostream>

#include "maxscore.h"

void hkSetMaxScore(unsigned int size) {
	hooks::max_score::maxScore = size;
	std::cout << "[+] Chart max score: " << hooks::max_score::maxScore << std::endl;
}

DWORD jmp_lr2body_4A8680 = 0x4A8680;
__declspec(naked) unsigned int trampSetMaxScore() {

	__asm {
		pushfd
		pushad
		push edi // max score is is in edi
		call hkSetMaxScore
		add esp, 4
		popad
		popfd
		// end hook
		add esp, 4
		mov dword ptr[esi + 4], 0
		jmp jmp_lr2body_4A8680
	}
}

void hooks::max_score::Setup() {
	mem::HookFn((char*)0x4A8679, (char*)trampSetMaxScore, 7);
}