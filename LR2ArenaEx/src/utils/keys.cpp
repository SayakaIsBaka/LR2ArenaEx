#include <dinput.h>
#include <iostream>
#include <string> 

#include "keys.h"

utils::keys::Key utils::keys::ParseKey(unsigned long cbData, void* lpvData, DeviceType type) { // TODO: handle midi
	unsigned char* arr;
	unsigned long size = cbData;
	Key res;
	res.type = DeviceType::NONE;

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
		return "Button " + std::to_string(key.value);
	}
	else if (key.type == DeviceType::KEYBOARD) {
		return keyToString.at(key.value);
	}
	else if (key.type == DeviceType::MIDI) {
		return ""; // TODO
	}
	return "";
}