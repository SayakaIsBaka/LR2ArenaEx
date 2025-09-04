#include <utils/mem.h>
#include <utils/keys.h>
#include <gui/gui.h>
#include <gui/graph.h>
#include <iostream>
#include <client/client.h>
#include <MinHook.h>

#include "midi.h"
#include "maniac.h"

void CheckKeyAndProcess(DWORD msg, utils::keys::Key key, std::function<void()> onMatch) {
	if (key.type == utils::keys::DeviceType::MIDI) {
		if ((msg & 0xFF) >> 4 == 9 && (msg & 0xFF00) >> 8 == key.value)
			onMatch();
	}
}

DWORD __cdecl hkParseMidiMessage(DWORD msg) {
	if (gui::waitingForKeyPress != utils::keys::BindingType::NONE) {
		auto parsedKey = utils::keys::ParseKey(0, (uintptr_t*)msg, utils::keys::DeviceType::MIDI);
		if (parsedKey.type != utils::keys::DeviceType::NONE) {
			if (utils::keys::BindKey(gui::waitingForKeyPress, parsedKey))
				gui::keySelected = true;
			else
				gui::keyAlreadyBound = parsedKey;
		}
	}
	else {
		if (hooks::maniac::itemModeEnabled && client::state.peers[client::state.remoteId].ready) {
			CheckKeyAndProcess(msg, utils::keys::bindings[utils::keys::BindingType::ITEM_TRIGGER].key, hooks::maniac::UseItem);
		}
		CheckKeyAndProcess(msg, utils::keys::bindings[utils::keys::BindingType::MENU_TOGGLE].key, [] { gui::showMenu = !gui::showMenu; });
		CheckKeyAndProcess(msg, utils::keys::bindings[utils::keys::BindingType::GRAPH_TOGGLE].key, [] { gui::graph::showGraph = !gui::graph::showGraph; });
	}

	if (gui::showMenu && gui::muteGameInputs) {
		return 0;
	}
	return hooks::midi::oParseMidiMessage(msg);
}

void hooks::midi::Setup() {
	oParseMidiMessage = (ParseMidiMessage)0x4BD740;
	if (MH_CreateHookEx((LPVOID)oParseMidiMessage, &hkParseMidiMessage, &oParseMidiMessage) != MH_OK)
	{
		std::cout << "[!] Error hooking ParseMidiMessage" << std::endl;
		return;
	}

	if (MH_QueueEnableHook(MH_ALL_HOOKS) || MH_ApplyQueued() != MH_OK)
	{
		std::cout << "[!] Error enabling ParseMidiMessage hook" << std::endl;
		return;
	}
}