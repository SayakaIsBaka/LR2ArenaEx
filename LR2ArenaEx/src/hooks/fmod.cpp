#include <utils/mem.h>
#include <utils/misc.h>
#include <iostream>

#include "fmod.h"

void hooks::fmod::PlayItemSound(void *sound) {
	if (systemObj != NULL && sound != NULL && channelGroup != NULL) {
		void* channel = NULL;
		if (PlaySound(systemObj, -1, sound, false, &channel) == 0) {
			SetChannelGroup(channel, channelGroup);
		}
	}
}

void hooks::fmod::InitDefaultSounds() {
	auto pathItemGetSound = (utils::GetLR2BasePath() / "LR2files" / "Sound" / "lr2" / "o-open.wav").u8string();
	auto pathItemUpgradeSound = (utils::GetLR2BasePath() / "LR2files" / "Sound" / "lr2" / "o-change.wav").u8string();
	auto pathItemReceivedSound = (utils::GetLR2BasePath() / "LR2files" / "Sound" / "lr2" / "f-close.wav").u8string();
	auto pathItemSendSound = (utils::GetLR2BasePath() / "LR2files" / "Sound" / "lr2" / "f-open.wav").u8string();

	if (CreateSound(systemObj, pathItemGetSound.c_str(), FMOD_LOOP_OFF | FMOD_ACCURATETIME | FMOD_HARDWARE, NULL, &itemGetSound)) {
		std::cout << "[!] Error loading item get sound" << std::endl;
		return;
	}

	if (CreateSound(systemObj, pathItemUpgradeSound.c_str(), FMOD_LOOP_OFF | FMOD_ACCURATETIME | FMOD_HARDWARE, NULL, &itemUpgradeSound)) {
		std::cout << "[!] Error loading item upgrade sound" << std::endl;
		return;
	}

	if (CreateSound(systemObj, pathItemReceivedSound.c_str(), FMOD_LOOP_OFF | FMOD_ACCURATETIME | FMOD_HARDWARE, NULL, &itemReceivedSound)) {
		std::cout << "[!] Error loading item received sound" << std::endl;
		return;
	}

	if (CreateSound(systemObj, pathItemSendSound.c_str(), FMOD_LOOP_OFF | FMOD_ACCURATETIME | FMOD_HARDWARE, NULL, &itemSendSound)) {
		std::cout << "[!] Error loading item send sound" << std::endl;
		return;
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
	if (itemGetSound != NULL)
		ReleaseSound(itemGetSound);
}