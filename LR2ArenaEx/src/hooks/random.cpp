#include <utils/mem.h>

#include "random.h"

void hooks::random::UpdateRandom() {
	EnterCriticalSection(&RandomCriticalSection);
	for (int i = 6; i >= 0; i--) {
		current_random[i] = GetRandomNumber(i);
		fprintf(stdout, "Random number %d: %d\n", i, current_random[i]);
	}
	LeaveCriticalSection(&RandomCriticalSection);
}

void swap_two_lanes(unsigned int* random, int i, int j) {
	unsigned int tmp = random[i];
	random[i] = random[j];
	random[j] = tmp;
}

void mirror_random(unsigned int* random) {
	for (int i = 0; i < 7; i++) {
		random[i] = 8 - random[i];
	}
}

// This is hooking the call to the RNG generator which is why the function is kinda odd at first
int hkRandom(int range, unsigned char* memory) {
	EnterCriticalSection(&hooks::random::RandomCriticalSection);
	int rand = hooks::random::current_random[range];
	LeaveCriticalSection(&hooks::random::RandomCriticalSection);
	if (range == 1 && hooks::random::random_flip) { // last call to trampRandom
		unsigned int* random_addr = (unsigned int*)(memory + 476);
		fprintf(stdout, "flipping random at address %p\n", random_addr);

		if (rand == 1) {
			swap_two_lanes(random_addr, 5, 6);
		}
		mirror_random(random_addr);
		if (rand == 1) {
			swap_two_lanes(random_addr, 5, 6); // neutralize swap made by LR2 later
		}
	}
	return rand;
}

__declspec(naked) void trampRandom() {

	__asm {
		// hook ecx = ptr ptr bms path
		pushfd
		push ecx
		push edx
		push ebx
		push esp
		push ebp
		push esi
		push edi
		// args
		push esp // random is at [esp+476]
		push edi
		call hkRandom
		add esp, 8
		//
		pop edi
		pop esi
		pop ebp
		pop esp
		pop ebx
		pop edx
		pop ecx
		popfd
		ret
		// end hook
	}
}

void hooks::random::Setup() {
	InitializeCriticalSection(&RandomCriticalSection);
	mem::HookFn((char*)0x4B4698, (char*)trampRandom, 5);
}

void hooks::random::Destroy() {
	DeleteCriticalSection(&RandomCriticalSection);
}