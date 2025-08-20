#pragma once

#include <unordered_map>
#include <overlay/overlay.h>
#include <dinput.h>
#include <utils/keys.h>
#include <ImGui/imgui.h>

namespace gui {
	namespace graph {
		enum class graphType {
			SCORE,
			BP,
			MAX_COMBO
		};

		inline bool showGraph = false;
		inline graph::graphType sortType(graph::graphType::SCORE);
		inline std::unordered_map<overlay::LR2_TYPE, ImVec2> graphDim = {
			{overlay::LR2_TYPE::LR2_HD, ImVec2(150, 400)},
			{overlay::LR2_TYPE::LR2_SD, ImVec2(100, 200)},
		};

		void Render();
	}
}