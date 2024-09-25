#pragma once

#include <iostream>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

namespace overlay {
	namespace dx9hook {
		constexpr char* d3dPattern = "\x33\xC0\xC7\x06\x00\x00\x00\x00\x89\x86\x00\x00\x00\x00\x89\x86";
		constexpr char* d3dMask = "xxxx????xx????xx?";
		constexpr uintptr_t internal_resolution = 0x7A3B50;

		typedef long(__stdcall* EndScene)(LPDIRECT3DDEVICE9);
		inline EndScene oEndScene;
		inline WNDPROC oWndProcHandler = NULL;

		typedef long(__cdecl* ShowCursor)(int);
		inline ShowCursor oShowCursor;

		inline std::string d3dName = "d3d9.dll";
		inline bool init = false;

		void HookDX9();
	}
}