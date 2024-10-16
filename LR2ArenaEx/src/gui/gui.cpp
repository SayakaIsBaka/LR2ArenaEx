#include <ImGui/imgui.h>
#include <server/server.h>
#include <client/client.h>

#include "gui.h"
#include "mainwindow.h"

void gui::Render() {
	if (ImGui::Begin("LR2ArenaEx", &showMenu, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
        if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Client"))
            {
                ImGui::SeparatorText("Connect");
                ImGui::InputTextWithHint("##Username", "Username", client::username, IM_ARRAYSIZE(client::username));
                ImGui::InputTextWithHint("##Server", "Server", client::host, IM_ARRAYSIZE(client::host));
                ImGui::BeginDisabled(strlen(client::host) == 0 || strlen(client::username) == 0);
                {
                    if (ImGui::Button("Connect")) {
                        client::Connect(client::host, client::username);
                        main_window::lines.clear();
                    }
                    ImGui::EndDisabled();
                }
                ImGui::EndTabItem();
                if (client::connected) {
                    ImGui::Separator();
                    main_window::Render();
                }
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
