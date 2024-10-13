#include <ImGui/imgui.h>
#include <server/server.h>
#include <client/client.h>

#include "gui.h"

void gui::Render() {
	if (ImGui::Begin("LR2ArenaEx", &showMenu, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
        if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Client"))
            {
                ImGui::SeparatorText("Settings");
                ImGui::InputTextWithHint("##Username", "Username", client::username, IM_ARRAYSIZE(client::username));
                ImGui::InputTextWithHint("##Server", "Server", client::host, IM_ARRAYSIZE(client::host));
                ImGui::BeginDisabled(strlen(client::host) == 0 || strlen(client::username) == 0);
                {
                    if (ImGui::Button("Connect"))
                        client::Connect(client::host, client::username);
                    ImGui::EndDisabled();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Server"))
            {
                ImGui::SeparatorText("Server");
                ImGui::BeginDisabled(server::started);
                {
                    if (ImGui::Button("Start"))
                        server::Start();
                    ImGui::EndDisabled();
                }
                ImGui::BeginDisabled(!server::started);
                {
                    if (ImGui::Button("Stop"))
                        server::Stop();
                    ImGui::EndDisabled();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

	}
	ImGui::End();
}
