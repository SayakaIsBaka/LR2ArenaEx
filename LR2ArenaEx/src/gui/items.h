#pragma once

#include <ImGui/imgui.h>
#include <overlay/overlay.h>

namespace gui {
	namespace items {
		inline ImFont* bigIconFont = nullptr;

		inline std::unordered_map<overlay::LR2_TYPE, ImVec2> buttonDim = {
			{overlay::LR2_TYPE::LR2_HD, ImVec2(75, 75)},
			{overlay::LR2_TYPE::LR2_SD, ImVec2(50, 50)},
		};

		void Render();
	}
}