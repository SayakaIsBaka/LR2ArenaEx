#include <utils/msgpack_utils.h>
#include <client/client.h>
#include <hooks/random.h>
#include <cstdio>
#include <utils/misc.h>
#include <fonts/IconsFontAwesome6.h>
#include <ImGui/ImGuiNotify.hpp>
#include <hooks/maniac.h>

#include "mainwindow.h"
#include "gui.h"
#include "widgets.h"
#include "itemsettings.h"

void SendMsg(std::string s) {
    client::Send(network::ClientToServer::CTS_MESSAGE, s);
}

// Weird tricks where we regroup all consecutive user messages into a blob to reduce processing
void gui::main_window::AddToLogWithUser(std::string s, network::Address id) {
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
            std::string username = value.username;
            if (key == client::state.host) {
                username = ICON_FA_CROWN " " + username;
            }
            if (value.ready && client::state.peers[client::state.host].selectedHash == value.selectedHash) {
                username = ICON_FA_CIRCLE_CHECK " " + username;
            }
            ImGui::Selectable(username.c_str());
            if (client::state.host == client::state.remoteId && ImGui::BeginPopupContextItem())
            {
                ImGui::TextDisabled("Selected user: %s", value.username.c_str());
                if (ImGui::MenuItem("Give host")) {
                    network::Address userId = key;
                    client::Send(network::ClientToServer::CTS_SET_HOST, msgpack_utils::pack(userId));
                }
                if (ImGui::MenuItem("Kick")) {
                    network::Address userId = key;
                    client::Send(network::ClientToServer::CTS_KICK_USER, msgpack_utils::pack(userId));
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

        if (ImGui::Button(ICON_FA_LINK)) {
            ImGui::SetClipboardText(("http://www.dream-pro.info/~lavalse/LR2IR/search.cgi?mode=ranking&bmsmd5=" + client::state.selectedSongRemote.hash).c_str());
            ImGui::InsertNotification({ ImGuiToastType::Info, 3000, "Copied LR2IR link to clipboard!" });
        }
        gui::widgets::Tooltip("Copy LR2IR link to clipboard");

        auto buttonWidth = ImGui::CalcTextSize(ICON_FA_LINK).x + ImGui::GetStyle().FramePadding.x * 2;
        auto fontSize = ImGui::GetFontSize();
        auto gapSize = fontSize / 2.0F;

        ImGui::SameLine(0.0F, gapSize);
        ImGui::PushItemWidth(mainViewDim[overlay::lr2type].x - (fontSize * 3) - buttonWidth - gapSize);
        ImGui::InputText("Title", (char*)client::state.selectedSongRemote.title.c_str(), client::state.selectedSongRemote.title.size(), ImGuiInputTextFlags_ReadOnly);
        ImGui::PopItemWidth();

        ImGui::PushItemWidth(mainViewDim[overlay::lr2type].x - (fontSize * 3));
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
            if (ImGui::BeginTabItem("Settings##Lobby"))
            {
                ImGui::SeparatorText("Player settings");
                ImGui::Checkbox("Enable random flip", &hooks::random::random_flip);
                ImGui::EndTabItem();

                ImGui::SeparatorText("Lobby settings");
                ImGui::BeginDisabled(!(client::state.host == client::state.remoteId));
                {
                    ImGui::Checkbox("Enable item / ojama mode", &hooks::maniac::itemModeEnabled);
                    ImGui::SameLine(); widgets::HelpMarker("Throw modifiers at your opponents while playing to add some spice!");
                    if (ImGui::Button("Item settings..."))
                        ImGui::OpenPopup("Item settings");
                    gui::item_settings::Render();
                    ImGui::EndDisabled();
                }
            }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();
        ImGui::EndGroup();
    }
}