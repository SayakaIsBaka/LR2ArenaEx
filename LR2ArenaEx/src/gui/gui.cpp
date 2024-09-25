#include <ImGui/imgui.h>

#include "gui.h"

void gui::Render() {
	if (ImGui::Begin("LR2ArenaEx", &showMenu, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
		ImGui::SeparatorText("Settings");
		static char str1[128] = "";
		ImGui::InputTextWithHint("##Username", "Username", str1, IM_ARRAYSIZE(str1));
		ImGui::SameLine();
		ImGui::Button("Set");
	}
	ImGui::End();
}
