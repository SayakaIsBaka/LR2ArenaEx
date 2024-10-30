#include <ImGui/imgui.h>
#include <fonts/IconsFontAwesome6.h>

#include "widgets.h"

void gui::widgets::Tooltip(const char* desc) {
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void gui::widgets::HelpMarker(const char* desc)
{
    ImGui::TextDisabled(ICON_FA_CIRCLE_INFO);
    Tooltip(desc);
}