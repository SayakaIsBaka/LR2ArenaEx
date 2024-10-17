#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <Garnet.h>
#include <overlay/overlay.h>
#include <ImGui/imgui.h>

namespace gui {
	namespace main_window {
		struct LogMessage {
			std::string msg;
			bool isSystemMsg;
		};

		inline std::vector<LogMessage> lines;
		inline char inputBuf[256];
		inline bool scrollToBottom = false;

		inline std::unordered_map<overlay::LR2_TYPE, ImVec2> userListDim = {
			{overlay::LR2_TYPE::LR2_HD, ImVec2(150, 0)},
			{overlay::LR2_TYPE::LR2_SD, ImVec2(60, 0)},
		};

		inline std::unordered_map<overlay::LR2_TYPE, ImVec2> mainViewDim = {
			{overlay::LR2_TYPE::LR2_HD, ImVec2(300, 400)},
			{overlay::LR2_TYPE::LR2_SD, ImVec2(230, 200)},
		};

		void Render();
		void ProcessInput();
		void AddToLog(std::string s);
		void AddToLogWithUser(std::string s, Garnet::Address id);
	}
}