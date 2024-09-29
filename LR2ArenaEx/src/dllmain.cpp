#include <iostream>
#include <framework.h>
#include <enet/enet.h>

#include "utils/mem.h"
#include "overlay/overlay.h"

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		if (enet_initialize() != 0) {
			std::cout << "[!] Error initalizing ENet" << std::endl;
			return false;
		}
		atexit(enet_deinitialize);

		auto* hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)overlay::Setup, hModule, 0, nullptr);
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