#include <utils/mem.h>
#include <utils/keys.h>
#include <gui/gui.h>
#include <iostream>

#include "midi.h"
#include "maniac.h"

DWORD __cdecl hkParseMidiMessage(DWORD msg) {
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