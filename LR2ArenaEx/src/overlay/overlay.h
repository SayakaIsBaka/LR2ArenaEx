#pragma once

#include <framework.h>

namespace overlay {
	enum class LR2_TYPE {
		LR2_SD = 1,
		LR2_HD = 2
	};
	struct LR2_RESOLUTION {
		unsigned int x = 0;
		unsigned int y = 0;
	};

	inline uintptr_t moduleBase;
	inline LR2_TYPE lr2type = LR2_TYPE::LR2_HD;
	inline LR2_RESOLUTION lr2res;

	DWORD WINAPI Setup(HMODULE hModule);
}