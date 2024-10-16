#pragma once

#include <vector>
#include <string>
#include <Garnet.h>

namespace gui {
	namespace main_window {
		inline std::vector<std::string> lines;
		inline char inputBuf[256];

		void Render();
		void ProcessInput();
		void AddToLog(std::string s);
		void AddToLogWithUser(std::string s, Garnet::Address id);
	}
}