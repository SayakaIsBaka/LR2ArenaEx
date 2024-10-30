#include <config/config.h>
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