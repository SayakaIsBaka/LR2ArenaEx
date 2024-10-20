#include <msgpack/msgpack.hpp>
#include <client/client.h>
#include <hooks/random.h>
#include <cstdio>
#include <utils/misc.h>
#include <fonts/IconsFontAwesome6.h>
#include <ImGui/ImGuiNotify.hpp>

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

// Weird tricks where we regroup all consecutive user messages into a blob to reduce processing
void gui::main_window::AddToLogWithUser(std::string s, Garnet::Address id) {
    auto username = client::state.peers[id].username;
    LogMessage msg;
    msg.msg = username + ": " + s + "\n";
    msg.isSystemMsg = false;
    if (lines.size() == 0 || lines[lines.size() - 1].isSystemMsg)
        lines.push_back(msg);
    else
        lines[lines.size() - 1].msg.append(msg.msg);

    if (!gui::showMenu) { // Show notification if main menu is not shown
        ImGuiToast toast(ImGuiToastType::Info, 3000);
        toast.setTitle("%s", username.c_str());
        toast.setContent("%s", s.c_str());
        ImGui::InsertNotification(toast);
    }
}

void gui::main_window::AddToLog(std::string s) { // AddToLog is basically only called for system messages
    LogMessage msg;
    msg.msg = s;
    msg.isSystemMsg = true;
    lines.push_back(msg);

    if (!gui::showMenu) { // Show notification if main menu is not shown
        std::string notifText = msg.msg;
        notifText.erase(0, 4);

        if (msg.msg.rfind("[#] ", 0) == 0) {
            ImGui::InsertNotification({ ImGuiToastType::Info, 3000, "%s", notifText.c_str() });
        }
        else if (msg.msg.rfind("[!] ", 0) == 0) {
            ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "%s", notifText.c_str() });
        }
    }
}

void gui::main_window::ProcessInput() {
    char* s = inputBuf;
    utils::StrTrim(s);
    if (s[0]) {
        AddToLogWithUser(s, client::state.remoteId);
        SendMsg(s);
    }
    strncpy(s, "", IM_ARRAYSIZE(inputBuf));
    scrollToBottom = true;
}

// Mostly taken from the Console and Simple layout ImGui demos
void gui::main_window::Render() {
    ImGui::SeparatorText("Lobby");
    {
        ImGui::BeginChild("Users", userListDim[overlay::lr2type], ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX);
        for (const auto& [key, value] : client::state.peers) {
            if (key == client::state.host) {
                ImGui::Selectable((ICON_FA_CROWN + std::string(" ") + value.username).c_str());
            }
            else
                ImGui::Selectable(value.username.c_str());
            if (client::state.host == client::state.remoteId && ImGui::BeginPopupContextItem())
            {
                ImGui::TextDisabled("Selected user: %s", value.username.c_str());
                if (ImGui::MenuItem("Give host")) {
                    Garnet::Address userId = key;
                    client::Send(network::ClientToServer::CTS_SET_HOST, msgpack::pack(userId));
                }
                if (ImGui::MenuItem("Kick")) {
                    Garnet::Address userId = key;
                    client::Send(network::ClientToServer::CTS_KICK_USER, msgpack::pack(userId));
                }
                ImGui::EndPopup();
            }
        }
        ImGui::EndChild();
    }
    ImGui::SameLine();
    {
        ImGui::BeginGroup();
        ImGui::BeginChild("Main view", mainViewDim[overlay::lr2type], ImGuiChildFlags_AutoResizeX);

        ImGui::PushItemWidth(mainViewDim[overlay::lr2type].x - (ImGui::GetFontSize() * 3));
        ImGui::InputText("Title", (char*)client::state.selectedSongRemote.title.c_str(), client::state.selectedSongRemote.title.size(), ImGuiInputTextFlags_ReadOnly);
        ImGui::InputText("Artist", (char*)client::state.selectedSongRemote.artist.c_str(), client::state.selectedSongRemote.artist.size(), ImGuiInputTextFlags_ReadOnly);
        ImGui::InputText("Path", (char*)client::state.selectedSongRemote.path.c_str(), client::state.selectedSongRemote.path.size(), ImGuiInputTextFlags_ReadOnly);
        ImGui::PopItemWidth();

        ImGui::Separator();
        if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Chat"))
            {
                const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
                if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
                    ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + mainViewDim[overlay::lr2type].x - 20);
                    for (const auto item : lines)
                    {
                        if (item.msg.empty())
                            continue;
                        ImVec4 color;
                        bool has_color = false;
                        if (item.msg.rfind("[#] ", 0) == 0) {
                            color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f);
                            has_color = true;
                        }
                        else if (item.msg.rfind("[!] ", 0) == 0) {
                            color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
                            has_color = true;
                        }

                        if (has_color)
                            ImGui::PushStyleColor(ImGuiCol_Text, color);
                        ImGui::TextUnformatted(item.msg.c_str(), item.msg.c_str() + item.msg.size());
                        if (has_color)
                            ImGui::PopStyleColor();
                    }
                    ImGui::PopTextWrapPos();

                    if (scrollToBottom || (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
                        ImGui::SetScrollHereY(1.0f);
                    scrollToBottom = false;

                    ImGui::PopStyleVar();
                }
                ImGui::EndChild();
                ImGui::Separator();

                bool reclaim_focus = false;
                ImGui::PushItemWidth(mainViewDim[overlay::lr2type].x - (ImGui::GetFontSize() * 3));
                if (ImGui::InputText("##Input", inputBuf, IM_ARRAYSIZE(inputBuf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll)) {
                    ProcessInput();
                    reclaim_focus = true;
                }
                ImGui::PopItemWidth();

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