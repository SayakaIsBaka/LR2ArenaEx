#pragma once

#include <ImGui/imgui.h>
#include <unordered_map>
#include <overlay/overlay.h>

namespace gui {
	inline bool showMenu = false;
	inline bool muteGameInputs = true;

	inline bool waitingForKeyPress = false;
	inline bool keySelected = false;

	inline std::unordered_map<overlay::LR2_TYPE, ImVec2> fileDialogDim = {
		{overlay::LR2_TYPE::LR2_HD, ImVec2(700, 500)},
		{overlay::LR2_TYPE::LR2_SD, ImVec2(500, 300)},
	};

	void Render();
}