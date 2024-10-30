#pragma once

#include <utils/keys.h>
#include <dinput.h>

namespace hooks {
	namespace maniac {
		inline utils::keys::Key itemKeyBind(utils::keys::DeviceType::KEYBOARD, DIK_BACKSPACE); // Default binding

		void SaveToConfigFile();
		void LoadConfig(std::string type, std::string value);
	}
}