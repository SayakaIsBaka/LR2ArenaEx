#include <utils/mem.h>
#include <config/config.h>
#include <iostream>

#include "fmod.h"

void hooks::fmod::SaveToConfigFile() {
	config::SetConfigValue("item_sound_volume", std::to_string(volume));
	config::SaveConfig();
}

void hooks::fmod::LoadConfig(std::string volume) {
	try {
		int intVol = std::stoi(volume);

		if (intVol < 0 || intVol > 100)
			throw std::invalid_argument("Invalid value");

		hooks::fmod::volume = intVol;
		if (channelGroup != NULL) { // If channel group is already created (extremely unlikely)
			SetVolume(channelGroup, (float)hooks::fmod::volume / 100.0f);
		}
	}
	catch (std::exception e) {
		std::cout << "[!] Error parsing config file for sound settings, falling back to default" << std::endl;
	}
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

void hooks::fmod::InitDefaultSounds() {
	for (auto& [key, val] : soundEffects) {
		if (CreateSound(systemObj, val.defaultPath.c_str(), FMOD_LOOP_OFF | FMOD_ACCURATETIME | FMOD_HARDWARE, NULL, &val.soundObject)) {
			std::cout << "[!] Error loading the following SFX: " << val.name << std::endl;
		}
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
				hooks::fmod::InitDefaultSounds();
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