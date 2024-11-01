#include <hooks/maniac.h>
#include <fonts/fa_solid_900.hpp>

#include "items.h"

void DisplayItem(const char* text) {
	if (gui::items::bigIconFont != nullptr) {
		ImGui::PushFont(gui::items::bigIconFont);
		ImGui::Button(text, gui::items::buttonDim[overlay::lr2type]);
		ImGui::PopFont();
	}
	else {
		ImGui::Button(text, gui::items::buttonDim[overlay::lr2type]);
	}
}

void gui::items::Render() {
	if (hooks::maniac::itemModeEnabled) {
		if (ImGui::Begin("Item", 0, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (hooks::maniac::currentItem.rolledItemId < 0) {
				ImGui::BeginDisabled();
				ImGui::Button("Empty", buttonDim[overlay::lr2type]);
				ImGui::EndDisabled();
			}
			else {
				auto i = hooks::maniac::currentItem.level + 2;
				ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(i / 7.0f, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(i / 7.0f, 0.8f, 0.8f));
				DisplayItem(hooks::maniac::items[hooks::maniac::currentItem.rolledItemId].icon.c_str());
				ImGui::PopStyleColor(3);
			}
			if (!hooks::maniac::activeItems.empty()) {
				ImGui::SameLine();
				ImGui::BeginChild("##ItemsDetails", ImVec2(0, 0), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
				{
					ImGui::SeparatorText("Current effects");
					for (const auto& [key, value] : hooks::maniac::activeItems) {
						ImGui::Text("%s (%02d:%02d)", hooks::maniac::items[key.rolledItemId].icon.c_str(), value / 60000, value / 1000);
					}
					ImGui::EndChild();
				}
			}
		}
		ImGui::End();
	}
}