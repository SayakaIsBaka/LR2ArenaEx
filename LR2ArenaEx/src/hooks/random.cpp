#include <utils/mem.h>

#include "random.h"

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
	int rand = hooks::random::GetRandomNumber(range);
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

int hkGetRandom(int maxNum) {
	if (hooks::random::received_random) {
		return hooks::random::current_seed;
	} else {
		int num = hooks::random::GetRandomNumber(maxNum);
		fprintf(stdout, "Generated seed: %d\n", num);
		return num;
	}
}

BOOL hkErrorLogFmtAdd(const char* format, unsigned int gp) {
	if (hooks::random::received_random) { // Rewrite here as well in case the player is not the host and tried to gbattle, same behaviour as gbattle (aka seed not writte in replay on first run)
		*(int*)(gp + 506736) = hooks::random::current_seed;
	}
	int num = *(int*)(gp + 506736);
	hooks::random::current_seed = num;
	fprintf(stdout, "Random seed: %d\n", hooks::random::current_seed);
	return hooks::random::ErrorLogFmtAdd(format, num);
}

void hooks::random::Setup() {
	mem::HookFn((char*)0x4B4698, (char*)trampRandom, 5);
	mem::HookFn((char*)0x4B07D3, (char*)hkGetRandom, 5); // Hook seed generation
	mem::HookFn((char*)0x4B080E, (char*)hkErrorLogFmtAdd, 5); // Hook logging call after seed generation
	mem::WriteMemory((char*)0x4B0802, "\x89\xEA\x90\x90\x90\x90", 6); // Change param to ErrorLogFmtAdd from struct member to pointer to struct
}

void hooks::random::Destroy() {
}