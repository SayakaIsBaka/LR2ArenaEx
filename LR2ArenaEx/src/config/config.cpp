#include <iostream>
#include <utils/misc.h>
#include <client/client.h>

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
}

void config::SaveConfig() {
	auto confPath = utils::GetConfigPath();

	mINI::INIFile file(confPath);
	file.write(ini, true);
}