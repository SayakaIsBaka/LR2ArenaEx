#include <utils/mem.h>
#include <utils/keys.h>
#include <gui/gui.h>
#include <gui/graph.h>
#include <iostream>
#include <client/client.h>

#include "midi.h"
#include "maniac.h"

void CheckKeyAndProcess(DWORD msg, utils::keys::Key key, std::function<void()> onMatch) {
	if (key.type == utils::keys::DeviceType::MIDI) {
		if ((msg & 0xFF) >> 4 == 9 && (msg & 0xFF00) >> 8 == key.value)
			onMatch();
	}
}

DWORD __cdecl hkParseMidiMessage(DWORD msg) {
	if (hooks::maniac::itemModeEnabled && client::state.peers[client::state.remoteId].ready) {
		CheckKeyAndProcess(msg, hooks::maniac::itemKeyBind, hooks::maniac::UseItem);
	}
	CheckKeyAndProcess(msg, gui::menuKeyBind, [] { gui::showMenu = !gui::showMenu; });
	CheckKeyAndProcess(msg, gui::graph::graphKeyBind, [] { gui::graph::showGraph = !gui::graph::showGraph; });
	if (gui::waitingForKeyPress) {
		auto parsedKey = utils::keys::ParseKey(0, (uintptr_t*)msg, utils::keys::DeviceType::MIDI);
		if (parsedKey.type != utils::keys::DeviceType::NONE) {
			hooks::maniac::itemKeyBind = parsedKey;
			gui::keySelected = true;
		}
	}
	if (gui::showMenu && gui::muteGameInputs) {
		return 0;
	}
	return hooks::midi::oParseMidiMessage(msg);
}

void hooks::midi::Setup() {
	oParseMidiMessage = (ParseMidiMessage)mem::TrampHook((char*)0x4BD740, (char*)hkParseMidiMessage, 6);
}