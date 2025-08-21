#include <iostream>
#include <utils/misc.h>
#include <client/client.h>
#include <hooks/maniac.h>
#include <hooks/fmod.h>
#include <gui/gui.h>
#include <gui/graph.h>

#include "config.h"

void config::LoadConfig() {
	auto confPath = utils::GetConfigPath();
	std::cout << "[i] Loading config from the following file: " << confPath << std::endl;

	mINI::INIFile file(confPath);
	file.read(ini);

	auto username = ini.get("config").get("username");
	if (!username.empty() && username.size() < 128)
		strcpy_s(client::username, 128, username.c_str());

	auto host = ini.get("config").get("host");
	if (!host.empty() && host.size() < 128)
		strcpy_s(client::host, 128, host.c_str());

	auto bindingsConfig = ini.get("bindings");
	utils::keys::LoadConfig(bindingsConfig);

	auto itemVolume = ini.get("config").get("item_sound_volume");
	auto sfxConfig = ini.get("sfx");
	hooks::fmod::LoadConfig(itemVolume, sfxConfig);

	auto muteInputs = ini.get("config").get("muteInputs");
	if (!muteInputs.empty()) {
		gui::muteGameInputs = muteInputs == "true" ? true : false;
	}

	auto automaticGraph = ini.get("config").get("automaticGraph");
	if (!automaticGraph.empty()) {
		gui::graph::automaticGraph = automaticGraph == "true" ? true : false;
	}
}

void config::SetConfigValue(std::string key, std::string val, std::string section) {
	config::ini[section][key] = val;
}

void config::RemoveConfigValue(std::string section, std::string key) {
	if (config::ini.has(section) && config::ini[section].has(key))
		config::ini[section].remove(key);
}

void config::SaveConfig() {
	auto confPath = utils::GetConfigPath();

	mINI::INIFile file(confPath);
	file.write(ini, true);
}