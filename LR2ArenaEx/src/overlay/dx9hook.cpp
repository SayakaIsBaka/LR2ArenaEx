#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_impl_dx9.h>
#include <utils/mem.h>
#include <gui/gui.h>
#include <gui/imguistyle.h>
#include <windowsx.h>

#include "dx9hook.h"
#include "overlay.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT __stdcall hkWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (GetAsyncKeyState(VK_INSERT) & 1) {
		gui::showMenu = !gui::showMenu;
	}

	// Fix mouse scaling for non-standard resolutions, taken from https://github.com/tenaibms/LR2OOL/blob/master/src/graphics/gui.cpp
	LPARAM imgui_lParam = lParam;

	if (uMsg == WM_MOUSEMOVE || uMsg == WM_NCMOUSEMOVE) {
		RECT r;
		GetClientRect(hWnd, &r);
		int physical_resolution[2] = { r.right - r.left, r.bottom - r.top };

		float scaling_factor_x = (float)((uintptr_t*)overlay::dx9hook::internal_resolution)[0] / physical_resolution[0];
		float scaling_factor_y = (float)((uintptr_t*)overlay::dx9hook::internal_resolution)[1] / physical_resolution[1];

		POINT mouse_pos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		mouse_pos.x *= scaling_factor_x;
		mouse_pos.y *= scaling_factor_y;
		imgui_lParam = MAKELPARAM(mouse_pos.x, mouse_pos.y);
	}

	if (gui::showMenu)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, imgui_lParam);
		return true;
	}
	return CallWindowProc(overlay::dx9hook::oWndProcHandler, hWnd, uMsg, wParam, lParam);
}

void InitImGui(IDirect3DDevice9* pDevice) {
	std::cout << "[i] Direct3D device address: " << (int*)*(int*)pDevice << std::endl;
	D3DDEVICE_CREATION_PARAMETERS CP;
	pDevice->GetCreationParameters(&CP);
	HWND window = CP.hFocusWindow;
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
	overlay::dx9hook::init = true;
	gui::SetupImGuiStyle();
	std::cout << "[i] ImGui initialized" << std::endl;

	overlay::dx9hook::oWndProcHandler = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG)hkWndProc);
	std::cout << "[i] Original WndProc: " << overlay::dx9hook::oWndProcHandler << std::endl;
	return;
}

HRESULT __stdcall hkEndScene(IDirect3DDevice9* pDevice) {
	if (!overlay::dx9hook::init) InitImGui(pDevice);
	else {
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();

		if (gui::showMenu)
		{
			gui::Render();
		}

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}
	return overlay::dx9hook::oEndScene(pDevice);
}

// Courtesy of https://github.com/tenaibms/LR2OOL/blob/master/src/hooks/cursor.cpp
int __cdecl hkShowCursor(int enabled) {
	if (gui::showMenu)
		return overlay::dx9hook::oShowCursor(1);
	return overlay::dx9hook::oShowCursor(enabled);
}

void overlay::dx9hook::HookDX9() {
	char* d3dPointer = mem::ScanModIn(d3dPattern, d3dMask, d3dName);
	std::cout << "[i] D3D pointer: " << (int*)d3dPointer << std::endl;

	uintptr_t d3dDeviceAddr = mem::FindDMAAddy((uintptr_t)d3dPointer + 0x4, { 0x0 });
	std::cout << "[i] D3D device pointer: " << (int*)d3dDeviceAddr << std::endl;

	oEndScene = (EndScene)mem::TrampHook(((char**)d3dDeviceAddr)[42], (char*)hkEndScene, 7);
	std::cout << "[i] EndScene pointer: " << std::hex << ((int*)d3dDeviceAddr)[42] << std::endl;

	oShowCursor = (ShowCursor)mem::TrampHook((char*)0x4D09E0, (char*)hkShowCursor, 6); // Hook hide mouse cursor
}