#include <hooks/maniac.h>
#include <fonts/fa_solid_900.hpp>

#include "items.h"

void DisplayItem(const char* text) {
	if (gui::items::bigIconFont != nullptr) {
		ImGui::PushFont(gui::items::bigIconFont);
		ImGui::Button(text, ImVec2(40, 40));
		ImGui::PopFont();
	}
	else {
		ImGui::Button(text);
	}
}

void gui::items::Render() {
	if (hooks::maniac::itemModeEnabled) {
		if (ImGui::Begin("Items", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (hooks::maniac::currentItem.rolledItemId < 0) {
				ImGui::BeginDisabled();
				ImGui::Button("Empty", ImVec2(40, 40));
				ImGui::EndDisabled();
			}
			else {
				DisplayItem(hooks::maniac::items[hooks::maniac::currentItem.rolledItemId].icon.c_str());
			}
		}
		ImGui::End();
	}
}