#include <iostream>
#include <fstream>

#include "Injector.h"
#include "LaunchGame.h"

static void PrintError()
{
	std::cout << "Press \"Enter\" to continue..." << std::endl;
	std::cin.ignore();
}

int main()
{
	/**if (LaunchGame() != 0)
	{
		PrintError();
		return 1;
	}**/

	if (Injector() != 0)
	{
		PrintError();
		return 1;
	}

	return 0;
}