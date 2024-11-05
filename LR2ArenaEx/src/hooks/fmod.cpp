#include <utils/mem.h>
#include <config/config.h>
#include <iostream>
#include <ImGui/ImGuiNotify.hpp>

#include "fmod.h"

void hooks::fmod::SaveToConfigFile() {
	config::SetConfigValue("item_sound_volume", std::to_string(volume));
	for (const auto& [key, val] : soundEffects) {
		if (!val.customPath.empty())
			config::SetConfigValue(key, val.customPath, "sfx");
		else
			config::RemoveConfigValue("sfx", key);
	}
	config::SaveConfig();
}

void hooks::fmod::LoadConfig(std::string volume, mINI::INIMap<std::string> sfxConfig) {
	try {
		if (!volume.empty()) {
			int intVol = std::stoi(volume);

			if (intVol < 0 || intVol > 100)
				throw std::invalid_argument("Invalid value");

			hooks::fmod::volume = intVol;
			if (channelGroup != NULL) { // If channel group is already created (extremely unlikely)
				SetVolume(channelGroup, (float)hooks::fmod::volume / 100.0f);
			}
		}
	}
	catch (std::exception e) {
		std::cout << "[!] Error parsing config file for sound settings, falling back to default" << std::endl;
	}
	try {
		if (sfxConfig.size() != 0) {
			for (const auto& [key, val] : sfxConfig) {
				if (soundEffects.count(key) == 0) {
					std::cout << "[!] Following sound effect does not exist, skipping: " << key << std::endl;
					continue;
				}
				soundEffects[key].customPath = val;
				if (systemObj != NULL && soundEffects[key].soundObject != NULL) { // If sound object is already created (extremely unlikely)
					LoadSound(key, val);
				}
			}
		}
	}
	catch (std::exception e) {
		std::cout << "[!] Error parsing config file for custom SFXs, falling back to default" << std::endl;
	}
}

bool hooks::fmod::LoadSound(std::string id, std::string path) {
	if (soundEffects.count(id) == 0) {
		std::cout << "[!] Specified sound effect id does not exist" << std::endl;
		return false;
	}
	void* tmpSound = NULL;
	if (CreateSound(systemObj, path.c_str(), FMOD_LOOP_OFF | FMOD_ACCURATETIME | FMOD_HARDWARE, NULL, &tmpSound)) {
		std::cout << "[!] Error loading the following SFX: " << soundEffects[id].name << std::endl;
		return false;
	}
	if (soundEffects[id].soundObject != NULL)
		ReleaseSound(soundEffects[id].soundObject);
	soundEffects[id].soundObject = tmpSound;
	return true;
}

void hooks::fmod::LoadNewCustomSound(std::string id, std::string selectedFile) {
	if (!selectedFile.empty()) {
		if (LoadSound(id, selectedFile)) {
			soundEffects[id].customPath = selectedFile;
			SaveToConfigFile();
		}
		else
			ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Error loading the following file: %s", selectedFile.c_str() });
	}
}

void hooks::fmod::ResetAllCustomSounds() {
	for (auto& [key, val] : soundEffects) {
		val.customPath = "";
	}
	InitSounds(true);
	SaveToConfigFile();
}

void hooks::fmod::PlaySfx(std::string id) {
	if (soundEffects.count(id) == 0) {
		std::cout << "[!] Sound effect not found: " << id << std::endl;
	}
	auto sound = soundEffects[id].soundObject;
	if (systemObj != NULL && sound != NULL && channelGroup != NULL) {
		void* channel = NULL;
		if (PlaySound(systemObj, -1, sound, false, &channel) == 0) {
			SetChannelGroup(channel, channelGroup);
		}
	}
}

void hooks::fmod::InitSounds(bool forceDefault) {
	for (auto& [key, val] : soundEffects) {
		std::string path = val.defaultPath;
		if (!val.customPath.empty() && !forceDefault)
			path = val.customPath;

		LoadSound(key, path);
	}
}

void hooks::fmod::SetItemVolume(int volume) {
	if (channelGroup != NULL) {
		float vol = (float)volume / 100.0f;
		SetVolume(channelGroup, vol);
		SaveToConfigFile();
	}
}

int __stdcall hkFmodSystemUpdate(void *system) {
	int res = hooks::fmod::oFmodSystemUpdate(system);
	if (res == 0 && hooks::fmod::systemObj == NULL) { // if FMOD_OK, get system object handle and init LR2ArenaEx channel group
		hooks::fmod::systemObj = system;
		if (hooks::fmod::channelGroup == NULL) {
			if (hooks::fmod::CreateChannelGroup(hooks::fmod::systemObj, "LR2ArenaEx_ChannelGroup", &hooks::fmod::channelGroup)) {
				std::cout << "[!] Error creating FMOD channel group" << std::endl;
			}
			else {
				hooks::fmod::InitSounds(false);
				hooks::fmod::SetVolume(hooks::fmod::channelGroup, (float)hooks::fmod::volume / 100.0f); // Apply volume when group is created
				std::cout << "[i] Succesfully initialized FMOD channel group and sounds" << std::endl;
			}
		}
	}
	return res;
}

void hooks::fmod::Setup() {
	oFmodSystemUpdate = (FMOD_System_Update)mem::TrampHook((char*)0x4C625E, (char*)hkFmodSystemUpdate, 6);
}

void hooks::fmod::Destroy() {
	if (channelGroup != NULL)
		ReleaseChannelGroup(channelGroup);
	for (auto& [key, val] : soundEffects) {
		if (val.soundObject != NULL)
			ReleaseSound(val.soundObject);
	}
}