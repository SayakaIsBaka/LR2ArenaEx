#include <ImGui/imgui.h>
#include <server/server.h>
#include <client/client.h>
#include <hooks/maniac.h>
#include <overlay/dx9hook.h>

#include "gui.h"
#include "widgets.h"
#include "mainwindow.h"

void gui::Render() {
	if (ImGui::Begin("LR2ArenaEx", &showMenu, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus))
	{
        if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_None))
        {
            if (ImGui::BeginTabItem("Client"))
            {
                if (ImGui::CollapsingHeader("Connect", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::InputTextWithHint("##Username", "Username", client::username, IM_ARRAYSIZE(client::username));
                    ImGui::InputTextWithHint("##Server", "Server", client::host, IM_ARRAYSIZE(client::host));
                    ImGui::BeginDisabled(strlen(client::host) == 0 || strlen(client::username) == 0);
                    {
                        if (ImGui::Button("Connect##Button")) {
                            client::Connect(client::host, client::username);
                            main_window::lines.clear();
                        }
                        ImGui::EndDisabled();
                    }
                    ImGui::SameLine();
                    ImGui::BeginDisabled(!client::connected);
                    {
                        if (ImGui::Button("Disconnect")) {
                            client::Destroy();
                            client::Init();
                        }
                        ImGui::EndDisabled();
                    }
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
                ImGui::Checkbox("Auto-rotate host after each song", &server::autoRotateHost);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Settings"))
            {
                ImGui::SeparatorText("Settings");
                ImGui::Checkbox("Disable game inputs when overlay is shown", &gui::muteGameInputs);
                ImGui::SameLine(); widgets::HelpMarker("Does not affect the graph display in-game");

                ImGui::Text("Item send key: %s", utils::keys::toString(hooks::maniac::itemKeyBind).c_str());
                ImGui::SameLine();
                if (ImGui::Button("Bind"))
                    ImGui::OpenPopup("Bind item key");

                ImVec2 center = ImVec2((float)((uintptr_t*)overlay::dx9hook::internal_resolution)[0] / 2.0f, (float)((uintptr_t*)overlay::dx9hook::internal_resolution)[1] / 2.0f);
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

                if (ImGui::BeginPopupModal("Bind item key", NULL, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    waitingForKeyPress = true;
                    ImGui::Text("Press any button...");
                    ImGui::Separator();

                    if (keySelected || ImGui::Button("Cancel")) {
                        waitingForKeyPress = false;
                        keySelected = false;
                        hooks::maniac::SaveToConfigFile();
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

	}
	ImGui::End();
}
