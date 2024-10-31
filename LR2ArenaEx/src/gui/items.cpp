#include <hooks/maniac.h>
#include <fonts/fa_solid_900.hpp>

#include "items.h"

void DisplayItem(const char* text) {
	if (gui::items::bigIconFont != nullptr) {
		ImGui::PushFont(gui::items::bigIconFont);
		ImGui::Button(text);
		ImGui::PopFont();
	}
	else {
		ImGui::Button(text);
	}
}

void gui::items::Render() {
	if (hooks::maniac::itemModeEnabled) {
		if (ImGui::Begin("Items", &hooks::maniac::itemModeEnabled, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			ImGui::BeginDisabled();
			DisplayItem(ICON_FA_SQUARE);
			ImGui::EndDisabled();
		}
		ImGui::End();
	}
}