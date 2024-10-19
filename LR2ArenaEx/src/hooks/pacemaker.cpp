#include <utils/mem.h>

#include "pacemaker.h"

void hkPacemaker(unsigned char* memory) {
	memory += 0x4C;
	if (!hooks::pacemaker::pacemaker_address) {
		*(unsigned int*)memory = hooks::pacemaker::displayed_score;
		hooks::pacemaker::pacemaker_address = (unsigned int*)memory;
	}
}

void hkPacemakerDisplay(unsigned char* memory) {
	memory += 0xD8;
	if (!hooks::pacemaker::pacemaker_display_address) {
		*(unsigned int*)memory = hooks::pacemaker::displayed_score;
		hooks::pacemaker::pacemaker_display_address = (unsigned int*)memory;
	}
}

DWORD jmp_lr2body_4A880A = 0x4A880A;
__declspec(naked) unsigned int trampPacemaker() {

	__asm {
		// hook [ecx+4Ch] = pacemaker score address
		pushfd
		pushad
		push ecx
		call hkPacemaker
		add esp, 4
		popad
		popfd
		// end hook
		mov eax, 1
		jmp jmp_lr2body_4A880A
	}
}

DWORD jmp_lr2body_4A9F07 = 0x4A9F07;
__declspec(naked) unsigned int trampPacemakerDisplay() {

	__asm {
		// hook [esi+0D8h] = pacemaker display score address
		pushfd
		pushad
		push esi
		call hkPacemakerDisplay
		add esp, 4
		popad
		popfd
		// end hook
		add esp, 4
		jmp jmp_lr2body_4A9F07
	}
}

void hooks::pacemaker::Setup() {
	mem::HookFn((char*)0x4A8802, (char*)trampPacemaker, 8);
	mem::HookFn((char*)0x4A9F01, (char*)trampPacemakerDisplay, 6);
	mem::WriteMemory((LPVOID)0x4A9F7B, (char*)"\x90\x90\x90\x90\x90\x90", 6); // nop pacemaker display update at the end
}

void hooks::pacemaker::Destroy() { // Restore original bytes
	mem::WriteMemory((LPVOID)0x4A8802, (char*)"\x89\x41\x4C\xB8\x01\x00\x00\x00", 8); // mov[ecx + 4Ch], eax; mov eax, 1
	mem::WriteMemory((LPVOID)0x4A9F01, (char*)"\x89\x86\xD8\x00\x00\x00", 6); // mov [esi+0D8h], eax
	mem::WriteMemory((LPVOID)0x4A9F7B, (char*)"\x89\x96\xD8\x00\x00\x00", 6); // mov [esi+0D8h], edx
}