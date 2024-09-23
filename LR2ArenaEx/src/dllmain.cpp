#include <fstream>
#include <iostream>
#include <thread>
#include <cstdint>
#include <utility>
#include <windowsx.h>

#include <framework.h>
#include "utils/mem.h"

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_impl_dx9.h>

#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

#define DIRECTINPUT_VERSION 0x0700 // DirectInput 7!!! (August 2007 SDK required)
#include <dinput.h>
#pragma comment(lib, "Dinput.lib")
#pragma comment(lib, "Dxguid.lib")

#define DEBUG_CONSOLE_ENABLED

constexpr char* d3dPattern = "\x33\xC0\xC7\x06\x00\x00\x00\x00\x89\x86\x00\x00\x00\x00\x89\x86";
constexpr char* d3dMask = "xxxx????xx????xx?";
constexpr uintptr_t internal_resolution = 0x7A3B50;
std::string d3dName = "d3d9.dll";

uintptr_t moduleBase;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef long(__stdcall* EndScene)(LPDIRECT3DDEVICE9);
uintptr_t endSceneAddr;
EndScene oEndScene;
HWND window;
bool init = false;
bool g_ShowMenu = false;
WNDPROC oWndProcHandler = NULL;

typedef long(__stdcall* GetDeviceState)(IDirectInputDevice7*, DWORD, LPVOID);
GetDeviceState oGetDeviceState;

HRESULT __stdcall hkGetDeviceState(IDirectInputDevice7* pThis, DWORD cbData, LPVOID lpvData) {
	HRESULT result = oGetDeviceState(pThis, cbData, lpvData);
	if (result == DI_OK && g_ShowMenu) { // TODO: need a way to differenciate main menu (where we may want to disable inputs) and everything else
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

LRESULT __stdcall hkWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (GetAsyncKeyState(VK_INSERT) & 1) {
		g_ShowMenu = !g_ShowMenu;
		ShowCursor(g_ShowMenu);
	}

	// Fix mouse scaling for non-standard resolutions, taken from https://github.com/tenaibms/LR2OOL/blob/master/src/graphics/gui.cpp
	LPARAM imgui_lParam = lParam;

	if (uMsg == WM_MOUSEMOVE || uMsg == WM_NCMOUSEMOVE) {
		RECT r;
		GetClientRect(hWnd, &r);
		int physical_resolution[2] = { r.right - r.left, r.bottom - r.top };

		float scaling_factor_x = (float)((uintptr_t*)internal_resolution)[0] / physical_resolution[0];
		float scaling_factor_y = (float)((uintptr_t*)internal_resolution)[1] / physical_resolution[1];

		POINT mouse_pos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		mouse_pos.x *= scaling_factor_x;
		mouse_pos.y *= scaling_factor_y;
		imgui_lParam = MAKELPARAM(mouse_pos.x, mouse_pos.y);
	}

	if (g_ShowMenu)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, imgui_lParam);
		return true;
	}
	return CallWindowProc(oWndProcHandler, hWnd, uMsg, wParam, lParam);
}

// Return -1 to make the game think it worked but do not actually call ShowCursor, handle the logic ourselves
int __stdcall hkShowCursor(BOOL bShow) {
	return -1;
}

void InitImGui(IDirect3DDevice9* pDevice) {
	std::cout << "[i] Direct3D device address:" << (int*)*(int*)pDevice << std::endl;
	D3DDEVICE_CREATION_PARAMETERS CP;
	pDevice->GetCreationParameters(&CP);
	window = CP.hFocusWindow;
	std::cout << "[i] Window address: " << window << std::endl;

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = NULL;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.Fonts->AddFontDefault();

	bool win32_init = ImGui_ImplWin32_Init(window);
	bool dx9_init = ImGui_ImplDX9_Init(pDevice);
	std::cout << "[i] ImGui_Win32 init success: " << win32_init << std::endl;
	std::cout << "[i] ImGui_DX9 init success: " << dx9_init << std::endl;
	init = true;
	std::cout << "[i] ImGui initialized" << std::endl;

	oWndProcHandler = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG)hkWndProc);
	std::cout << "[i] Original WndProc: " << oWndProcHandler << std::endl;
	return;
}

HRESULT __stdcall hkEndScene(IDirect3DDevice9* pDevice) {
	if (!init) InitImGui(pDevice);
	else {
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();

		if (g_ShowMenu)
		{
			bool bShow = true;
			ImGui::ShowDemoWindow(&bShow);
		}

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}
	return oEndScene(pDevice);
}

BOOL HackDinput(HMODULE hModule) {
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

DWORD WINAPI HackThread(HMODULE hModule)
{
#ifdef DEBUG_CONSOLE_ENABLED
	AllocConsole();
	FILE* f = nullptr;
	freopen_s(&f, "CONOUT$", "w", stdout);

	std::cout << "--- LR2ArenaEx debugging console ---" << std::endl << std::endl;
#endif
	if((moduleBase = (uintptr_t)GetModuleHandle("LRHbody.exe")) == 0)
	{
		moduleBase = (uintptr_t)GetModuleHandle("LR2body.exe");
	}

	char* d3dPointer = mem::ScanModIn(d3dPattern, d3dMask, d3dName);
	std::cout << "[i] D3D pointer: " << (int*)d3dPointer << std::endl;
	int d3dDeviceAddr = mem::FindDMAAddy((uintptr_t)d3dPointer + 0x4, { 0x0 });
	std::cout << "[i] D3D device pointer: " << (int*)d3dDeviceAddr << std::endl;
	uintptr_t* d3d9Device = new uintptr_t[119];
	*&d3d9Device = (uintptr_t*)d3dDeviceAddr;
	oEndScene = (EndScene)mem::TrampHook((char*)d3d9Device[42], (char*)hkEndScene, 7);
	std::cout << "[i] EndScene pointer: " << (int*)d3d9Device[42] << std::endl;

	mem::HookFn((char*)0x4CBED0, (char*)hkShowCursor, 6); // Hook hide mouse cursor #1
	mem::HookFn((char*)0x4D0A43, (char*)hkShowCursor, 6); // Hook hide mouse cursor #2

	HackDinput(hModule);

#ifdef DEBUG_CONSOLE_ENABLED
	while (true)
	{
	}
	fclose(f);
	FreeConsole();
#endif

#pragma warning(push)
#pragma warning(disable : 6258)
	TerminateThread(GetCurrentThread(), 0);
#pragma warning(pop)
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		auto* hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, nullptr);
		if (hThread == nullptr)
		{
			std::cout << "[!] Couldn't create main thread" << std::endl;
			return FALSE;
		}

		CloseHandle(hThread);
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}