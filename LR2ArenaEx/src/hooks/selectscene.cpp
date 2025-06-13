#include <utils/mem.h>
#include <gui/graph.h>

#include "selectscene.h"

void hkSelectScene() {
	gui::graph::showGraph = false;
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

void hooks::select_scene::Setup() {
	mem::HookFn((char*)0x432006, (char*)trampSelectScene, 5);
}