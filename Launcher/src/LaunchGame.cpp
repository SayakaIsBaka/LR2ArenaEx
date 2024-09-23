#include "LaunchGame.h"
#include <Windows.h>
#include <fstream>
#include <iostream>

int LaunchGame()
{
	STARTUPINFO si{};
	PROCESS_INFORMATION pi{};
	
	if (CreateProcess("LRHbody.exe", nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi) != 1)
	{
		if (CreateProcess("LR2body.exe", nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi) != 1)
		{
			std::cout << "Couldn't launch exe\n";
			return 1;
		}
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return 0;
}