#include <ImGui/imgui.h>
#include <client/client.h>
#include <hooks/random.h>
#include <cstdio>
#include <utils/misc.h>

#include "mainwindow.h"

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
        //ExecCommand(s);
    }
    strncpy(s, "", IM_ARRAYSIZE(inputBuf));
}

void gui::main_window::Render() {
    ImGui::SeparatorText("Users");
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
        ImGui::BeginChild("item view", ImVec2(300, 300), ImGuiChildFlags_AutoResizeX); // Leave room for 1 line below us
        ImGui::Separator();
        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Chat"))
            {
                const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
                if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
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
                            ImGui::TextUnformatted(item.c_str());
                            if (has_color)
                                ImGui::PopStyleColor();
                        }
                    }

                    // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
                    // Using a scrollbar or mouse-wheel will take away from the bottom edge.
                    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                        ImGui::SetScrollHereY(1.0f);

                    ImGui::PopStyleVar();
                }
                ImGui::EndChild();
                ImGui::Separator();

                // Command-line
                bool reclaim_focus = false;
                bool sendMessage = ImGui::InputText("##Input", inputBuf, IM_ARRAYSIZE(inputBuf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll);
                ImGui::SameLine();
                sendMessage |= ImGui::Button("Send");
                if (sendMessage) {
                    ProcessInput();
                    reclaim_focus = true;
                }

                ImGui::SetItemDefaultFocus();
                if (reclaim_focus)
                    ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

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