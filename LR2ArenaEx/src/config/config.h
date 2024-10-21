#pragma once

#include <mini/ini.h>

namespace config {
	inline mINI::INIStructure ini;

	void LoadConfig();
	void SaveConfig();
}