#include "dinputhook.h"

#include <iostream>
#include <utils/mem.h>
#include <gui/gui.h>
#include <gui/graph.h>
#include <hooks/maniac.h>
#include <client/client.h>
#include <MinHook.h>

constexpr int debounceRate = 50;

void CheckKeyAndProcess(DWORD cbData, LPVOID lpvData, utils::keys::Key key, std::function<void()> onMatch) {
	if (cbData == sizeof(DIJOYSTATE) && key.type == utils::keys::DeviceType::CONTROLLER) {
		auto arr = ((LPDIJOYSTATE)lpvData)->rgbButtons;
		if (arr[key.value]) {
			if (overlay::dinputhook::heldKeys.find(key) == overlay::dinputhook::heldKeys.end()) {
				onMatch();
			}
			overlay::dinputhook::heldKeys[key] = debounceRate;
		}
	}
	else if (cbData == 256 && key.type == utils::keys::DeviceType::KEYBOARD) {
		if (((BYTE*)lpvData)[key.value]) {
			if (overlay::dinputhook::heldKeys.find(key) == overlay::dinputhook::heldKeys.end()) {
				onMatch();
			}
			overlay::dinputhook::heldKeys[key] = debounceRate;
		}
	}
	for (auto& [key, value] : overlay::dinputhook::heldKeys) {
		if (overlay::dinputhook::heldKeys.empty()) // Happens sometimes for some reason, wrapping operations with a mutex doesn't fix it
			return;
		value--;
		if (value <= 0)
			overlay::dinputhook::heldKeys.erase(key);
	}
}

HRESULT __stdcall hkGetDeviceState(IDirectInputDevice7* pThis, DWORD cbData, LPVOID lpvData) {
	HRESULT result = overlay::dinputhook::oGetDeviceState(pThis, cbData, lpvData);
	if (result == DI_OK) {
		if (gui::waitingForKeyPress != utils::keys::BindingType::NONE) {
			utils::keys::DeviceType type = utils::keys::DeviceType::NONE;
			if (cbData == sizeof(DIJOYSTATE)) // Controller device
				type = utils::keys::DeviceType::CONTROLLER;
			else if (cbData == 256) // Keyboard device
				type = utils::keys::DeviceType::KEYBOARD;
			if (type != utils::keys::DeviceType::NONE) {
				auto parsedKey = utils::keys::ParseKey(cbData, lpvData, type);
				if (parsedKey.type != utils::keys::DeviceType::NONE) {
					overlay::dinputhook::heldKeys[parsedKey] = debounceRate;
					if (utils::keys::BindKey(gui::waitingForKeyPress, parsedKey))
						gui::keySelected = true;
					else
						gui::keyAlreadyBound = parsedKey;
				}
			}
		}
		else {
			if (hooks::maniac::itemModeEnabled && client::state.peers[client::state.remoteId].ready) {
				CheckKeyAndProcess(cbData, lpvData, utils::keys::bindings[utils::keys::BindingType::ITEM_TRIGGER].key, hooks::maniac::UseItem);
			}
			CheckKeyAndProcess(cbData, lpvData, utils::keys::bindings[utils::keys::BindingType::MENU_TOGGLE].key, [] { gui::showMenu = !gui::showMenu; });
			CheckKeyAndProcess(cbData, lpvData, utils::keys::bindings[utils::keys::BindingType::GRAPH_TOGGLE].key, [] { gui::graph::showGraph = !gui::graph::showGraph; });
		}
		if (gui::showMenu && gui::muteGameInputs) {
			if (cbData == sizeof(DIMOUSESTATE2)) { // Mouse device
				((LPDIMOUSESTATE2)lpvData)->rgbButtons[0] = 0;
				((LPDIMOUSESTATE2)lpvData)->rgbButtons[1] = 0;
				((LPDIMOUSESTATE2)lpvData)->lZ = 0;
			}
			else if (cbData == 256) { // Keyboard device
				memset(lpvData, 0, 256);
			}
			/*
			else if (cbData == sizeof(DIJOYSTATE)) { // Controller device
				memset(lpvData, 0, cbData);
			}*/
		}
	}
	return result;
}

static overlay::dinputhook::GetDeviceState GetDeviceStatePtr7(HMODULE hModule) {
	IDirectInput7A* pDirectInput = NULL;
	typedef HRESULT(__stdcall* tDirectInputCreateEx)(HINSTANCE hinst,
		DWORD dwVersion,
		REFIID riidltf,
		LPVOID* ppvOut,
		LPUNKNOWN punkOuter);
	tDirectInputCreateEx DirectInputCreateEx = (tDirectInputCreateEx)GetProcAddress(hModule, "DirectInputCreateEx");
	if (DirectInputCreateEx == nullptr) {
		std::cout << "[!] DirectInputCreateEx not present in dinput.dll" << std::endl;
		return nullptr;
	}
	GUID IID_IDirectInput7A = { 0x9A4CB684,0x236D,0x11D3,0x8E,0x9D,0x00,0xC0,0x4F,0x68,0x44,0xAE };
	if (DirectInputCreateEx(GetModuleHandle(NULL), 0x0700, IID_IDirectInput7A, (LPVOID*)&pDirectInput, NULL) != DI_OK) {
		std::cout << "[!] DirectInputCreateEx failed" << std::endl;
		return nullptr;
	}

	LPDIRECTINPUTDEVICE7A lpdiMouse;
	GUID GUID_SysMouse = { 0x6F1D2B60, 0xD5A0, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 };
	GUID IID_IDirectInputDevice7A = { 0x57D7C6BC,0x2356,0x11D3,0x8E,0x9D,0x00,0xC0,0x4F,0x68,0x44,0xAE };
	if (pDirectInput->CreateDeviceEx(GUID_SysMouse, IID_IDirectInputDevice7A, (LPVOID*)&lpdiMouse, NULL) != DI_OK) {
		pDirectInput->Release();
		std::cout << "[!] Error creating DirectInput7 device" << std::endl;
		return nullptr;
	}

	void** vTable = reinterpret_cast<void**>(mem::FindDMAAddy((uintptr_t)lpdiMouse, { 0x0 }));

	lpdiMouse->Release();
	pDirectInput->Release();

	return reinterpret_cast<overlay::dinputhook::GetDeviceState>(vTable[9]);
}

static overlay::dinputhook::GetDeviceState GetDeviceStatePtr8(HMODULE hModule) {
	IDirectInput8A* pDirectInput = NULL;
	decltype(DirectInput8Create)* DirectInput8Create = reinterpret_cast<decltype(DirectInput8Create)>(GetProcAddress(hModule, "DirectInput8Create"));
	if (DirectInput8Create == nullptr) {
		std::cout << "[!] DirectInput8Create not present in dinput8.dll" << std::endl;
		return nullptr;
	}
	GUID IID_IDirectInput8A = { 0xBF798030,0x483A,0x4DA2,0xAA,0x99,0x5D,0x64,0xED,0x36,0x97,0x00 };
	if (DirectInput8Create(GetModuleHandle(NULL), 0x0800, IID_IDirectInput8A, (LPVOID*)&pDirectInput, NULL) != DI_OK) {
		std::cout << "[!] DirectInput8Create failed" << std::endl;
		return nullptr;
	}

	LPDIRECTINPUTDEVICE8A lpdiMouse;
	GUID GUID_SysMouse = { 0x6F1D2B60, 0xD5A0, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 };
	if (pDirectInput->CreateDevice(GUID_SysMouse, &lpdiMouse, NULL) != DI_OK) {
		pDirectInput->Release();
		std::cout << "[!] Error creating DirectInput8 device" << std::endl;
		return nullptr;
	}

	void** vTable = reinterpret_cast<void**>(mem::FindDMAAddy((uintptr_t)lpdiMouse, { 0x0 }));

	lpdiMouse->Release();
	pDirectInput->Release();

	return reinterpret_cast<overlay::dinputhook::GetDeviceState>(vTable[9]);
}

BOOL overlay::dinputhook::HookDinput(HMODULE hModule) {
	int ver = 0;
	HMODULE dinputHandle7 = GetModuleHandle("dinput.dll");
	if (dinputHandle7) ver = 7;
	HMODULE dinputHandle8 = GetModuleHandle("dinput8.dll");
	if (dinputHandle8) ver = 8;

	switch (ver) {
	case 7: oGetDeviceState = GetDeviceStatePtr7(dinputHandle7); break;
	case 8: oGetDeviceState = GetDeviceStatePtr8(dinputHandle8); break;
	default: std::cout << "[!] DirectInput not found" << std::endl; return false;
	}
	if (MH_CreateHookEx((LPVOID)oGetDeviceState, &hkGetDeviceState, &oGetDeviceState) != MH_OK)
	{
		std::cout << "[!] Error hooking GetDeviceState" << std::endl;
		return false;
	}

	if (MH_QueueEnableHook(MH_ALL_HOOKS) || MH_ApplyQueued() != MH_OK)
	{
		std::cout << ("[!] Error enabling dinput hooks") << std::endl;
		return false;
	}

	return true;
}