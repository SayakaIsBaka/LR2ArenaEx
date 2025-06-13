#include "gui.h"
#include "widgets.h"
#include "mainwindow.h"

#include <server/server.h>
#include <client/client.h>
#include <hooks/maniac.h>
#include <hooks/fmod.h>
#include <overlay/dx9hook.h>
#include <ImGui/ImGuiFileDialog.h>

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
                            client::Disconnect();
                            client::Destroy();
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

                static int volumeTmp = hooks::fmod::volume;
                ImGui::SliderInt("Item sounds volume", &volumeTmp, 0, 100, "%d%%", ImGuiSliderFlags_AlwaysClamp);
                if (ImGui::IsItemDeactivated() && volumeTmp != hooks::fmod::volume) { // Only update when the slider is not being dragged anymore
                    hooks::fmod::volume = volumeTmp;
                    hooks::fmod::SetItemVolume(hooks::fmod::volume);
                }

                if (ImGui::CollapsingHeader("Item SFX settings")) {
                    if (ImGui::BeginTable("ItemSfxTable", 3,
                        ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersH,
                        ImVec2(ImGui::GetFontSize() * 25.0f, ImGui::GetTextLineHeightWithSpacing() * 7))) {
                        ImGui::TableSetupScrollFreeze(1, 0);
                        ImGui::TableSetupColumn("");
                        ImGui::TableSetupColumn("Name");
                        ImGui::TableSetupColumn("File");
                        ImGui::TableHeadersRow();
                        for (const auto& [key, val] : hooks::fmod::soundEffects) {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            if (ImGui::Button((ICON_FA_FOLDER_OPEN "##" + key).c_str())) {
                                IGFD::FileDialogConfig config;
                                config.countSelectionMax = 1;
                                config.flags = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_DisableCreateDirectoryButton | ImGuiFileDialogFlags_CaseInsensitiveExtentionFiltering;
                                config.userDatas = IGFD::UserDatas(key.c_str());
                                ImGuiFileDialog::Instance()->OpenDialog("ChooseAudioFileDlg", "Select an audio file...", "Audio files (*.wav, *.ogg, *.mp3){.wav,.ogg,.mp3}", config);
                            }
                            ImGui::SameLine();
                            if (ImGui::Button((ICON_FA_PLAY "##" + key).c_str())) {
                                hooks::fmod::PlaySfx(key);
                            }
                            ImGui::TableNextColumn();
                            ImGui::Text("%s", val.name.c_str());
                            ImGui::TableNextColumn();
                            if (val.customPath.empty())
                                ImGui::TextDisabled("(Default)");
                            else
                                ImGui::Text("%s", val.customPath.c_str());
                        }
                        ImGui::EndTable();
                    }
                    if (ImGui::Button("Reset all SFXs")) {
                        hooks::fmod::ResetAllCustomSounds();
                    }
                }

                if (ImGui::CollapsingHeader("Bindings settings")) {
                    if (ImGui::BeginTable("BindingsTable", 3,
                        ImGuiTableFlags_ScrollX | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersH,
                        ImVec2(ImGui::GetFontSize() * 25.0f, 0))) {
                        ImGui::TableSetupScrollFreeze(1, 0);
                        ImGui::TableSetupColumn("Name");
                        ImGui::TableSetupColumn("Key");
                        ImGui::TableSetupColumn("");
                        ImGui::TableHeadersRow();
                        for (const auto& [key, val] : utils::keys::bindings) {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text("%s", val.name.c_str());
                            ImGui::TableNextColumn();
                            ImGui::Text("%s", utils::keys::toString(val.key).c_str());
                            ImGui::TableNextColumn();
                            if (ImGui::Button(std::string("Bind##" + val.id).c_str())) {
                                waitingForKeyPress = key;
                            }
                        }
                        ImGui::EndTable();
                    }
                }

                if (waitingForKeyPress != utils::keys::BindingType::NONE)
                    ImGui::OpenPopup("Bind key");

                ImVec2 center = ImVec2((float)((uintptr_t*)overlay::dx9hook::internal_resolution)[0] / 2.0f, (float)((uintptr_t*)overlay::dx9hook::internal_resolution)[1] / 2.0f);
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

                if (ImGui::BeginPopupModal("Bind key", NULL, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("Binding key for: %s", utils::keys::bindings[waitingForKeyPress].name.c_str());
                    ImGui::Text("Press any button...");
                    ImGui::Separator();

                    if (keySelected || ImGui::Button("Cancel")) {
                        waitingForKeyPress = utils::keys::BindingType::NONE;
                        keySelected = false;
                        utils::keys::SaveToConfigFile();
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, "", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA_FOLDER);
        ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".wav", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA_MUSIC);
        ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".ogg", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA_MUSIC);
        ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".mp3", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA_MUSIC);

        ImVec2 center = ImVec2((float)((uintptr_t*)overlay::dx9hook::internal_resolution)[0] / 2.0f, (float)((uintptr_t*)overlay::dx9hook::internal_resolution)[1] / 2.0f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGuiFileDialog::Instance()->Display("ChooseAudioFileDlg", ImGuiWindowFlags_NoCollapse, fileDialogDim[overlay::lr2type])) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePath = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string id;
                if (ImGuiFileDialog::Instance()->GetUserDatas())
                    id = std::string((const char*)ImGuiFileDialog::Instance()->GetUserDatas());
                hooks::fmod::LoadNewCustomSound(id, filePath);
            }
            ImGuiFileDialog::Instance()->Close();
        }
	}
	ImGui::End();
}
