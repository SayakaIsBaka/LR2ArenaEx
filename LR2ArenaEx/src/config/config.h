#pragma once

#include <mini/ini.h>

namespace config {
	inline mINI::INIStructure ini;

	void LoadConfig();
	void SaveConfig();
	void SetConfigValue(std::string key, std::string val, std::string section = "config");
	void RemoveConfigValue(std::string section, std::string key);
}