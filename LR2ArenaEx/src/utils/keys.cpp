#include <dinput.h>
#include <iostream>
#include <string> 

#include "keys.h"

utils::keys::Key ParseMidi(unsigned long msg) {
	uint8_t status = (msg & 0xFF) >> 4;
	if (status == 9) { // Note on
		uint8_t note = (msg & 0xFF00) >> 8;
		return utils::keys::Key(utils::keys::DeviceType::MIDI, note);
	}
	return utils::keys::Key(utils::keys::DeviceType::NONE, 0);
}

utils::keys::Key utils::keys::ParseKey(unsigned long cbData, void* lpvData, DeviceType type) {
	unsigned char* arr;
	unsigned long size = cbData;
	Key res;
	res.type = DeviceType::NONE;

	if (type == DeviceType::MIDI) {
		return ParseMidi((DWORD)lpvData); // Very different processing so treat it separately
	}
	if (type == DeviceType::CONTROLLER) {
		arr = ((LPDIJOYSTATE)lpvData)->rgbButtons;
		size = 32;
	}
	if (type == DeviceType::KEYBOARD) {
		arr = (unsigned char*)lpvData;
	}
	unsigned long i = 0;
	for (; i < size; i++) {
		if (arr[i] & 0xF0) // Found pressed key
			break;
	}
	if (i >= size) // No keys pressed
		return res;
	
	res.type = type;
	res.value = i;
	return res;
}

std::string utils::keys::toString(utils::keys::Key key) {
	if (key.type == DeviceType::CONTROLLER) {
		return "Button " + std::to_string(key.value) + " (Controller)";
	}
	else if (key.type == DeviceType::KEYBOARD) {
		try {
			return keyToString.at(key.value);
		}
		catch (std::out_of_range e) {
			return "[invalid value]";
		}
	}
	else if (key.type == DeviceType::MIDI) {
		int octave = (key.value / 12) - 2;
		std::string note = midiNotes[key.value % 12];
		return note + std::to_string(octave) + " (MIDI)";
	}
	return "";
}

void utils::keys::SaveToConfigFile() {
	for (const auto& [key, val] : bindings) {
		config::SetConfigValue(val.id + "_controller_type", std::to_string(static_cast<unsigned int>(val.key.type)), "bindings");
		config::SetConfigValue(val.id + "_keybind", std::to_string(val.key.value), "bindings");
	}
	config::SaveConfig();
}

void utils::keys::LoadConfig(mINI::INIMap<std::string> bindingsConfig) {
	for (auto& [key, val] : bindings) {
		try {
			auto type = bindingsConfig.get(val.id + "_controller_type");
			auto value = bindingsConfig.get(val.id + "_keybind");
			if (!type.empty() && !value.empty()) {
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

				val.key = utils::keys::Key(t, intVal);
			}
		}
		catch (std::exception e) {
			std::cout << "[!] Error parsing config file for " << val.name << ", falling back to default" << std::endl;
		}
	}
}

bool utils::keys::BindKey(BindingType bindingType, Key key) {
	for (const auto& [_, val] : bindings) {
		if (val.key == key) // If key already bound to another feature
			return false;
	}
	bindings[bindingType].key = key;
	return true;
}