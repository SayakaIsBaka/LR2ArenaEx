#include <iostream>
#include <utils/mem.h>
#include <gui/gui.h>

#include "dinputhook.h"

HRESULT __stdcall hkGetDeviceState(IDirectInputDevice7* pThis, DWORD cbData, LPVOID lpvData) {
	HRESULT result = overlay::dinputhook::oGetDeviceState(pThis, cbData, lpvData);
	if (result == DI_OK && gui::showMenu) { // TODO: need a way to differenciate main menu (where we may want to disable inputs) and everything else
		if (cbData == sizeof(DIMOUSESTATE2)) { // Mouse device
			((LPDIMOUSESTATE2)lpvData)->rgbButtons[0] = 0;
			((LPDIMOUSESTATE2)lpvData)->rgbButtons[1] = 0;
		}
		else if (cbData == 256) { // Keyboard device
			memset(lpvData, 0, 256);
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