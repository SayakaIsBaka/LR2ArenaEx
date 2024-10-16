#include <ImGui/imgui.h>
#include <client/client.h>
#include <hooks/random.h>
#include <cstdio>
#include <utils/misc.h>

#include "mainwindow.h"

void SendMsg(std::string s) {
    client::Send(network::ClientToServer::CTS_MESSAGE, s);
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
            if (key == client::state.host)
                ImGui::Selectable(("👑 " + value.username).c_str());
            else
                ImGui::Selectable(value.username.c_str());
        }
        ImGui::EndChild();
    }
    ImGui::SameLine();
    {
        ImGui::BeginGroup();
        ImGui::BeginChild("item view", ImVec2(300, 300), ImGuiChildFlags_AutoResizeX);
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
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();
        ImGui::EndGroup();
    }
}