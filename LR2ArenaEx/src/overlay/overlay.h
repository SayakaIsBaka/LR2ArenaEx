#pragma once

#include <framework.h>

namespace overlay {
	inline uintptr_t moduleBase;

	DWORD WINAPI Setup(HMODULE hModule);
}