#pragma once

#include <framework.h>
#define DIRECTINPUT_VERSION 0x0700 // DirectInput 7!!! (August 2007 SDK required)
#include <dinput.h>
#include <unordered_map>
#include <utils/keys.h>
#pragma comment(lib, "Dinput.lib")
#pragma comment(lib, "Dxguid.lib")

namespace overlay {
	namespace dinputhook {
		typedef long(__stdcall* GetDeviceState)(IDirectInputDevice7*, DWORD, LPVOID);
		inline GetDeviceState oGetDeviceState;

		inline std::unordered_map<utils::keys::Key, unsigned int> heldKeys;

		BOOL HookDinput(HMODULE hModule);
	}
}