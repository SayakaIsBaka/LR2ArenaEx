#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_win32.h>
#include <ImGui/imgui_impl_dx9.h>
#include <ImGui/implot.h>
#include <ImGui/ImGuiNotify.hpp>
#include <utils/mem.h>
#include <gui/gui.h>
#include <gui/graph.h>
#include <gui/items.h>
#include <gui/imguistyle.h>
#include <windowsx.h>
#include <fonts/noto_medium.hpp>
#include <fonts/fa_solid_900.hpp>
#include <fonts/IconsFontAwesome6.h>
#include <MinHook.h>
#include <optional>

#include "dx9hook.h"
#include "overlay.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool ShouldMuteMessage(UINT uMsg) {
	const UINT toMute[] = {
		WM_MOUSEHWHEEL,
		WM_MOUSEWHEEL,
		WM_SETCURSOR
	};
	for (const UINT m : toMute) {
		if (uMsg == m)
			return true;
	}
	return false;
}

LRESULT __stdcall hkWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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

	if (gui::showMenu || gui::graph::showGraph)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, imgui_lParam);
		if (gui::showMenu && gui::muteGameInputs && ShouldMuteMessage(uMsg))
			return true;
	}
	return CallWindowProc(overlay::dx9hook::oWndProcHandler, hWnd, uMsg, wParam, lParam);
}

void SetupFonts(ImGuiIO& io, int fontSize) {
	float mainFontSize = (float)fontSize;
	float iconFontSize = mainFontSize * 2.0f / 3.0f;

	ImVector<ImWchar> ranges;
	ImFontGlyphRangesBuilder builder;
	builder.AddText(u8"←→↑↓");
	builder.AddRanges(io.Fonts->GetGlyphRangesJapanese());
	builder.BuildRanges(&ranges);
	io.Fonts->AddFontFromMemoryCompressedTTF(noto_compressed_data, noto_compressed_size, mainFontSize, 0, ranges.Data);

	static const ImWchar iconsRanges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	ImFontConfig iconsConfig;
	iconsConfig.MergeMode = true;
	iconsConfig.PixelSnapH = true;
	iconsConfig.GlyphMinAdvanceX = iconFontSize;
	io.Fonts->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data, fa_solid_900_compressed_size, iconFontSize, &iconsConfig, iconsRanges);

	// Setup bigger size font for less aliased display
	iconsConfig.MergeMode = false;
	iconFontSize *= 4.0f;
	gui::items::bigIconFont = io.Fonts->AddFontFromMemoryCompressedTTF(fa_solid_900_compressed_data, fa_solid_900_compressed_size, iconFontSize, &iconsConfig, iconsRanges);

	io.Fonts->Build();
}

void InitImGui(IDirect3DDevice9* pDevice) {
	std::cout << "[i] Direct3D device address: " << (int*)*(int*)pDevice << std::endl;
	D3DDEVICE_CREATION_PARAMETERS CP;
	pDevice->GetCreationParameters(&CP);
	HWND window = CP.hFocusWindow;
	std::cout << "[i] Window address: " << window << std::endl;

	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	//io.IniFilename = NULL;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	int fontSize = overlay::lr2type == overlay::LR2_TYPE::LR2_HD ? 16 : 12;
	SetupFonts(io, fontSize);

	bool win32_init = ImGui_ImplWin32_Init(window);
	bool dx9_init = ImGui_ImplDX9_Init(pDevice);
	std::cout << "[i] ImGui_Win32 init success: " << win32_init << std::endl;
	std::cout << "[i] ImGui_DX9 init success: " << dx9_init << std::endl;
	overlay::dx9hook::init = true;
	gui::SetupImGuiStyle();
	std::cout << "[i] ImGui initialized" << std::endl;

	overlay::dx9hook::oWndProcHandler = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG)hkWndProc);
	std::cout << "[i] Original WndProc: " << overlay::dx9hook::oWndProcHandler << std::endl;

	overlay::dx9hook::lastKnownDevice = pDevice;
	return;
}

void ResetImGui(IDirect3DDevice9* pDevice)
{
	D3DDEVICE_CREATION_PARAMETERS params;
	pDevice->GetCreationParameters(&params);

	ImGui_ImplWin32_Shutdown();
	ImGui_ImplWin32_Init(params.hFocusWindow);

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplDX9_Init(pDevice);

	overlay::dx9hook::lastKnownDevice = pDevice;
}

void RenderNotifications() {
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f));

	ImGui::RenderNotifications();

	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(1);
}

HRESULT __stdcall hkEndScene(IDirect3DDevice9* pDevice) {
	if (!overlay::dx9hook::init) {
		InitImGui(pDevice);
		return overlay::dx9hook::oEndScene(pDevice);
	}

	if (overlay::dx9hook::lastKnownDevice != pDevice) ResetImGui(pDevice);

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();

	if (gui::showMenu)
	{
		gui::Render();
	}
	if (gui::graph::showGraph)
	{
		gui::graph::Render();
	}
	RenderNotifications();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	return overlay::dx9hook::oEndScene(pDevice);
}

HRESULT __stdcall hkResetScene(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pParams) {
	ImGui_ImplDX9_InvalidateDeviceObjects();
	auto res = overlay::dx9hook::oResetScene(pDevice, pParams);
	ImGui_ImplDX9_CreateDeviceObjects();
	return res;
}

// Courtesy of https://github.com/tenaibms/LR2OOL/blob/master/src/hooks/cursor.cpp
int __cdecl hkShowCursor(int enabled) {
	if (gui::showMenu || gui::graph::showGraph)
		return overlay::dx9hook::oShowCursor(1);
	return overlay::dx9hook::oShowCursor(enabled);
}

struct DxFuncs {
	overlay::dx9hook::EndScene endScene = nullptr;
	overlay::dx9hook::ResetScene resetScene = nullptr;
};
static std::optional<DxFuncs> GetDX9Pointers() {
	WNDCLASSEX windowClass;
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = DefWindowProc;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandle(NULL);
	windowClass.hIcon = NULL;
	windowClass.hCursor = NULL;
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = "dxWindow";
	windowClass.hIconSm = NULL;

	::RegisterClassEx(&windowClass);

	HWND window = ::CreateWindow(windowClass.lpszClassName, "LR2ArenaEx DirectX Window", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, windowClass.hInstance, NULL);

	HMODULE libD3D9;
	if ((libD3D9 = ::GetModuleHandle("d3d9.dll")) == NULL)
	{
		::DestroyWindow(window);
		::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return std::nullopt;
	}

	void* Direct3DCreate9;
	if ((Direct3DCreate9 = ::GetProcAddress(libD3D9, "Direct3DCreate9")) == NULL)
	{
		::DestroyWindow(window);
		::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return std::nullopt;
	}

	LPDIRECT3D9 direct3D9;
	if ((direct3D9 = ((LPDIRECT3D9(__stdcall*)(uint32_t))(Direct3DCreate9))(D3D_SDK_VERSION)) == NULL)
	{
		::DestroyWindow(window);
		::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return std::nullopt;
	}

	D3DPRESENT_PARAMETERS params;
	params.BackBufferWidth = 0;
	params.BackBufferHeight = 0;
	params.BackBufferFormat = D3DFMT_UNKNOWN;
	params.BackBufferCount = 0;
	params.MultiSampleType = D3DMULTISAMPLE_NONE;
	params.MultiSampleQuality = NULL;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	params.hDeviceWindow = window;
	params.Windowed = 1;
	params.EnableAutoDepthStencil = 0;
	params.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	params.Flags = NULL;
	params.FullScreen_RefreshRateInHz = 0;
	params.PresentationInterval = 0;

	LPDIRECT3DDEVICE9 device;
	if (direct3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, window, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_DISABLE_DRIVER_MANAGEMENT, &params, &device) < 0)
	{
		direct3D9->Release();
		::DestroyWindow(window);
		::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
		return std::nullopt;
	}

	std::cout << "[i] D3D device pointer: " << (int*)device << std::endl;

	DxFuncs funcs{};

	funcs.endScene = (decltype(funcs.endScene))(*(char***)device)[42];
	std::cout << "[i] EndScene pointer: " << std::hex << (*(int**)device)[42] << std::endl;

	funcs.resetScene = (decltype(funcs.resetScene))(*(char***)device)[16];
	std::cout << "[i] ResetScene pointer: " << std::hex << (*(int**)device)[16] << std::endl;

	device->Release();
	device = NULL;

	direct3D9->Release();
	direct3D9 = NULL;

	::DestroyWindow(window);
	::UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

	return funcs;
}

void overlay::dx9hook::HookDX9() {
	auto dx9Funcs = GetDX9Pointers();
	if (!dx9Funcs) {
		std::cout << "[!] Error getting DX9 pointers" << std::endl;
		return;
	}
	oEndScene = dx9Funcs->endScene;
	oResetScene = dx9Funcs->resetScene;

	if (MH_CreateHookEx((LPVOID)oEndScene, &hkEndScene, &oEndScene) != MH_OK)
	{
		std::cout << "[!] Error hooking EndScene" << std::endl;
		return;
	}

	if (MH_CreateHookEx((LPVOID)oResetScene, &hkResetScene, &oResetScene) != MH_OK)
	{
		std::cout << "[!] Error hooking ResetScene" << std::endl;
		return;
	}

	oShowCursor = (ShowCursor)0x4D09E0;
	if (MH_CreateHookEx((LPVOID)oShowCursor, &hkShowCursor, &oShowCursor) != MH_OK) // Hook hide mouse cursor
	{
		std::cout << "[!] Error hooking ShowCursor" << std::endl;
		return;
	}

	if (MH_QueueEnableHook(MH_ALL_HOOKS) || MH_ApplyQueued() != MH_OK)
	{
		std::cout << "[!] Error enabling DX9 hooks" << std::endl;
		return;
	}
}