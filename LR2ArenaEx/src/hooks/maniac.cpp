#include <config/config.h>
#include <utils/mem.h>
#include <iostream>

#include "maniac.h"

void hooks::maniac::SaveToConfigFile() {
	config::SetConfigValue("controller_type", std::to_string(static_cast<unsigned int>(itemKeyBind.type)));
	config::SetConfigValue("item_keybind", std::to_string(itemKeyBind.value));
	config::SaveConfig();
}

void hooks::maniac::LoadConfig(std::string type, std::string value) {
	try {
		int intType = std::stoi(type);
		int intVal = std::stoi(value);
		utils::keys::DeviceType t;

		if (intVal < 0)
			throw std::invalid_argument("Invalid value");

		switch (intType) {
		case 2:
			t = utils::keys::DeviceType::KEYBOARD;
			if (intVal >= 0xEE)
				throw std::invalid_argument("Invalid value");
			break;
		case 3:
			t = utils::keys::DeviceType::CONTROLLER;
			if (intVal >= 32)
				throw std::invalid_argument("Invalid value");
			break;
		case 4:
			t = utils::keys::DeviceType::MIDI;
			if (intVal >= 128)
				throw std::invalid_argument("Invalid value");
			break;
		default:
			throw std::invalid_argument("Invalid device type");
		}

		itemKeyBind = utils::keys::Key(t, intVal);
	}
	catch (std::exception e) {
		std::cout << "[!] Error parsing config file for key bindings, falling back to default" << std::endl;
	}
}

void hooks::maniac::ResetState() {
	currentCombo = 0;
	rolledItemId = -1;
}

void hkResetCombo() {
	hooks::maniac::currentCombo = 0;
}

void hkUpdateCombo() {
	hooks::maniac::currentCombo++;
	if (hooks::maniac::currentCombo == 100) { // TODO: edit threshold
		hooks::maniac::rolledItemId = hooks::maniac::dist(hooks::maniac::rng);
	}
}

DWORD jmp_lr2body_406440 = 0x406440;
__declspec(naked) unsigned int trampResetCombo() {

	__asm {
		pushfd
		pushad
		call hkResetCombo
		popad
		popfd
		// end hook
		mov [edi + 97990h], edx
		add esp, 4
		jmp jmp_lr2body_406440
	}
}

DWORD jmp_lr2body_40640A = 0x40640A;
__declspec(naked) unsigned int trampUpdateCombo() {

	__asm {
		pushfd
		pushad
		call hkUpdateCombo
		popad
		popfd
		// end hook
		add [edi + 97990h], ecx
		add esp, 4
		jmp jmp_lr2body_40640A
	}
}

void hooks::maniac::Setup() {
	// Init RNG
	rng = std::mt19937(dev());
	dist = std::uniform_int_distribution<std::mt19937::result_type>(0, items.size());

	mem::HookFn((char*)0x406404, (char*)trampUpdateCombo, 6);
	mem::HookFn((char*)0x40643A, (char*)trampResetCombo, 6);
}