#pragma once

#include <iostream>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

namespace overlay {
	namespace dx9hook {
		constexpr char* d3dPattern = "\x33\xC0\xC7\x06\x00\x00\x00\x00\x89\x86\x00\x00\x00\x00\x89\x86";
		constexpr char* d3dMask = "xxxx????xx????xx?";
		inline int canvas_resolution[2] = { 640, 480 };
		inline int output_resolution[2] = { 640, 480 };

		typedef long(__stdcall* EndScene)(LPDIRECT3DDEVICE9);
		inline EndScene oEndScene;
		typedef long(__stdcall* ResetScene)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
		inline ResetScene oResetScene;
		typedef long(__stdcall* Present)(LPDIRECT3DSWAPCHAIN9, const RECT* pSourceRect, const RECT*, HWND, const RGNDATA*, DWORD);
		inline Present oPresent;
		inline HWND hWnd = NULL;
		inline WNDPROC oWndProcHandler = NULL;
		inline IDirect3DDevice9* lastKnownDevice = NULL;
		inline int rtMax = 1;

		typedef long(__cdecl* ShowCursor)(int);
		inline ShowCursor oShowCursor;

		inline std::string d3dName = "d3d9.dll";
		inline bool init = false;

		void HookDX9();
	}
}