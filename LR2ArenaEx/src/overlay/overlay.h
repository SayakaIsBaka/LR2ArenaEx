#pragma once

#include <framework.h>

namespace overlay {
	enum class LR2_TYPE {
		LR2_SD = 1,
		LR2_HD = 2
	};

	inline uintptr_t moduleBase;
	inline LR2_TYPE lr2type = LR2_TYPE::LR2_HD;

	DWORD WINAPI Setup(HMODULE hModule);
}