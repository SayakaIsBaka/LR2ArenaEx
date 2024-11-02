#include <hooks/maniac.h>
#include <ImGui/imgui.h>
#include <overlay/dx9hook.h>

#include "itemsettings.h"
#include "widgets.h"

void gui::item_settings::Render() {
    ImVec2 center = ImVec2((float)((uintptr_t*)overlay::dx9hook::internal_resolution)[0] / 2.0f, (float)((uintptr_t*)overlay::dx9hook::internal_resolution)[1] / 2.0f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Item settings", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::SeparatorText("Items");
        static auto itemsTmp = hooks::maniac::items;
        static int thresholdMultTmp = hooks::maniac::thresholdMult * 100;

        if (hooks::maniac::settingsRemoteUpdated) { // Update temp objects if settings were synced from the server
            itemsTmp = hooks::maniac::items;
            thresholdMultTmp = hooks::maniac::thresholdMult * 100;
            hooks::maniac::settingsRemoteUpdated = false;
        }

        if (ImGui::BeginTable("itemTable", 5, ImGuiTableFlags_BordersInnerV)) {
            ImGui::TableSetupColumn("Item");
            ImGui::TableSetupColumn("Lv1", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Lv2", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Lv3", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Weight", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (auto &item : itemsTmp) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%s %s", item.icon.c_str(), item.name.c_str());
                ImGui::TableNextColumn();
                if (item.name == "Acceleration") {
                    ImGui::Text("Accel");
                    ImGui::TableNextColumn();
                    ImGui::Text("Deccel");
                    ImGui::TableNextColumn();
                    ImGui::Text("Random");
                }
                else {
                    ImGui::DragInt(("##Lv1" + item.name).c_str(), (int*)&item.lv1, 1.0f, 0, 100, "%d%%", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::TableNextColumn();
                    ImGui::DragInt(("##Lv2" + item.name).c_str(), (int*)&item.lv2, 1.0f, 0, 100, "%d%%", ImGuiSliderFlags_AlwaysClamp);
                    ImGui::TableNextColumn();
                    ImGui::DragInt(("##Lv3" + item.name).c_str(), (int*)&item.lv3, 1.0f, 0, 100, "%d%%", ImGuiSliderFlags_AlwaysClamp);
                }
                ImGui::TableNextColumn();
                ImGui::DragInt(("##Weight" + item.name).c_str(), (int*)&item.weight, 1.0f, 0, 1000, "%d", ImGuiSliderFlags_AlwaysClamp);
            }

            ImGui::EndTable();
        }
        ImGui::SeparatorText("Threshold");
        ImGui::SliderInt("Item threshold", &thresholdMultTmp, 0, 100, "%d%%", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SameLine(); widgets::HelpMarker("Indicates after how much combo a player gets an item; given as a percentage of the total number of notes in the selected chart");

        ImGui::Separator();
        if (ImGui::Button("Save")) {
            hooks::maniac::items = itemsTmp;
            hooks::maniac::thresholdMult = (float)thresholdMultTmp / 100.0f;

            hooks::maniac::SendItemSettings();

            hooks::maniac::UpdateItemWeights();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            itemsTmp = hooks::maniac::items;
            thresholdMultTmp = hooks::maniac::thresholdMult * 100;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}