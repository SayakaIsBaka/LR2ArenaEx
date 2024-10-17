#include <ImGui/imgui.h>
#include <client/client.h>
#include <hooks/random.h>
#include <cstdio>
#include <utils/misc.h>
#include <fonts/IconsFontAwesome6.h>

#include "mainwindow.h"
#include "gui.h"

void SendMsg(std::string s) {
    client::Send(network::ClientToServer::CTS_MESSAGE, s);
}

void HelpMarker(const char* desc)
{
    ImGui::TextDisabled(ICON_FA_CIRCLE_INFO);
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void gui::main_window::AddToLogWithUser(std::string s, Garnet::Address id) {
    auto username = client::state.peers[id].username;
    lines.push_back(username + ": " + s);
}

void gui::main_window::AddToLog(std::string s) {
    lines.push_back(s);
}

void gui::main_window::ProcessInput() {
    char* s = inputBuf;
    utils::StrTrim(s);
    if (s[0]) {
        AddToLogWithUser(s, client::state.remoteId);
        SendMsg(s);
    }
    strncpy(s, "", IM_ARRAYSIZE(inputBuf));
}

// Mostly taken from the Console and Simple layout ImGui demos
void gui::main_window::Render() {
    ImGui::SeparatorText("Lobby");
    {
        ImGui::BeginChild("Users", ImVec2(150, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
        for (const auto& [key, value] : client::state.peers) {
            if (key == client::state.host) {
                ImGui::Selectable((ICON_FA_CROWN + std::string(" ") + value.username).c_str());
            }
            else
                ImGui::Selectable(value.username.c_str());
        }
        ImGui::EndChild();
    }
    ImGui::SameLine();
    {
        ImGui::BeginGroup();
        ImGui::BeginChild("Main view", ImVec2(300, 400), ImGuiChildFlags_AutoResizeX);

        ImGui::InputText("Hash", (char*)client::state.selectedSongRemote.hash.c_str(), client::state.selectedSongRemote.hash.size(), ImGuiInputTextFlags_ReadOnly);
        ImGui::InputText("Title", (char*)client::state.selectedSongRemote.title.c_str(), client::state.selectedSongRemote.title.size(), ImGuiInputTextFlags_ReadOnly);
        ImGui::InputText("Artist", (char*)client::state.selectedSongRemote.artist.c_str(), client::state.selectedSongRemote.artist.size(), ImGuiInputTextFlags_ReadOnly);

        ImGui::Separator();
        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Chat"))
            {
                const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
                if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
                    ImGuiListClipper clipper;
                    clipper.Begin(lines.size());
                    while (clipper.Step()) {
                        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                        {
                            auto item = lines[i];
                            ImVec4 color;
                            bool has_color = false;
                            if (item.rfind("[#] ", 0) == 0) {
                                color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f);
                                has_color = true;
                            }
                            else if (item.rfind("[!] ", 0) == 0) {
                                color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
                                has_color = true;
                            }

                            if (has_color)
                                ImGui::PushStyleColor(ImGuiCol_Text, color);
                            ImGui::TextWrapped("%s\n", item.c_str());
                            if (has_color)
                                ImGui::PopStyleColor();
                        }
                    }

                    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                        ImGui::SetScrollHereY(1.0f);

                    ImGui::PopStyleVar();
                }
                ImGui::EndChild();
                ImGui::Separator();

                bool reclaim_focus = false;
                if (ImGui::InputText("##Input", inputBuf, IM_ARRAYSIZE(inputBuf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll)) {
                    ProcessInput();
                    reclaim_focus = true;
                }

                ImGui::SetItemDefaultFocus();
                if (reclaim_focus)
                    ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

                ImGui::SameLine();
                if (ImGui::Button("Send") && !reclaim_focus) // Prevent double send if pressing Enter and clicking the button on the same frame (unlikely)
                    ProcessInput();

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Settings"))
            {
                ImGui::Checkbox("Enable random flip", &hooks::random::random_flip);
                ImGui::Checkbox("Disable game inputs when overlay is shown", &gui::muteGameInputs);
                ImGui::SameLine(); HelpMarker("Does not affect the graph display in-game");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();
        ImGui::EndGroup();
    }
}