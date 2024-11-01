#include "dinputhook.h"

#include <iostream>
#include <utils/mem.h>
#include <utils/keys.h>
#include <gui/gui.h>
#include <hooks/maniac.h>
#include <client/client.h>

HRESULT __stdcall hkGetDeviceState(IDirectInputDevice7* pThis, DWORD cbData, LPVOID lpvData) {
	HRESULT result = overlay::dinputhook::oGetDeviceState(pThis, cbData, lpvData);
	if (result == DI_OK) {
		if (hooks::maniac::itemModeEnabled && client::state.peers[client::state.remoteId].ready) {
			if (cbData == sizeof(DIJOYSTATE) && hooks::maniac::itemKeyBind.type == utils::keys::DeviceType::CONTROLLER) {
				auto arr = ((LPDIJOYSTATE)lpvData)->rgbButtons;
				if (arr[hooks::maniac::itemKeyBind.value])
					hooks::maniac::UseItem();
			} else if (cbData == 256 && hooks::maniac::itemKeyBind.type == utils::keys::DeviceType::KEYBOARD) {
				if (((BYTE*)lpvData)[hooks::maniac::itemKeyBind.value])
					hooks::maniac::UseItem();
			}
		}
		if (gui::waitingForKeyPress) {
			utils::keys::DeviceType type = utils::keys::DeviceType::NONE;
			if (cbData == sizeof(DIJOYSTATE)) // Controller device
				type = utils::keys::DeviceType::CONTROLLER;
			else if (cbData == 256) // Keyboard device
				type = utils::keys::DeviceType::KEYBOARD;
			if (type != utils::keys::DeviceType::NONE) {
				auto parsedKey = utils::keys::ParseKey(cbData, lpvData, type);
				if (parsedKey.type != utils::keys::DeviceType::NONE) {
					hooks::maniac::itemKeyBind = parsedKey;
					gui::keySelected = true;
				}
			}
		}
		if (gui::showMenu && gui::muteGameInputs) {
			if (cbData == sizeof(DIMOUSESTATE2)) { // Mouse device
				((LPDIMOUSESTATE2)lpvData)->rgbButtons[0] = 0;
				((LPDIMOUSESTATE2)lpvData)->rgbButtons[1] = 0;
			}
			else if (cbData == 256) { // Keyboard device
				memset(lpvData, 0, 256);
			}
			else if (cbData == sizeof(DIJOYSTATE)) { // Controller device
				memset(lpvData, 0, cbData);
			}
		}
	}
	return result;
}

BOOL overlay::dinputhook::HookDinput(HMODULE hModule) {
	IDirectInput7* pDirectInput = NULL;
	if (DirectInputCreateEx(hModule, DIRECTINPUT_VERSION, IID_IDirectInput7A, (LPVOID*)&pDirectInput, NULL) != DI_OK) {
		std::cout << "[i] DirectInputCreateEx failed" << std::endl;
		return false;
	}

	LPDIRECTINPUTDEVICE7 lpdiMouse;
	if (pDirectInput->CreateDeviceEx(GUID_SysMouse, IID_IDirectInputDevice7A, (LPVOID*)&lpdiMouse, NULL) != DI_OK) {
		pDirectInput->Release();
		std::cout << "[!] Error creating DirectInput device" << std::endl;
		return false;
	}

	uintptr_t vTable = mem::FindDMAAddy((uintptr_t)lpdiMouse, { 0x0 });

	std::cout << "[i] DInput device pointer: " << std::hex << lpdiMouse << std::endl;
	std::cout << "[i] DInput vTable pointer: " << std::hex << vTable << std::endl;
	std::cout << "[i] GetDeviceState pointer: " << std::hex << ((int*)vTable)[9] << std::endl;
	oGetDeviceState = (GetDeviceState)mem::TrampHook(((char**)vTable)[9], (char*)hkGetDeviceState, 7);

	lpdiMouse->Release();
	pDirectInput->Release();

	return true;
}