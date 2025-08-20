#include <utils/mem.h>
#include <iostream>

#include "currentnotes.h"

void hkSetCurrentNotes(unsigned int count) {
	hooks::current_notes::currentNotes = count;
	std::cout << "[+] Chart current notes: " << hooks::current_notes::currentNotes << std::endl;
}

DWORD jmp_lr2body_406175 = 0x406175;
__declspec(naked) unsigned int trampSetCurrentNotes() {

	__asm {
		pushfd
		pushad
		push ecx // current notes is is in ecx
		call hkSetCurrentNotes
		add esp, 4
		popad
		popfd
		// end hook
		cmp ecx, dword ptr ds : [eax + 0x97A40]
		jmp jmp_lr2body_406175
	}
}

void hooks::current_notes::Setup() {
	mem::Hook((char*)0x40616F, (char*)trampSetCurrentNotes, 6);
}