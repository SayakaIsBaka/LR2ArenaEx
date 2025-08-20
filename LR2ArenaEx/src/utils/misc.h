#pragma once

#include <network/structs.h>
#include <filesystem>
#include <vector>

namespace utils {
	unsigned int CalculateExScore(network::Score score);
	void StrTrim(char* s);
	std::string SJISToUTF8(const std::string& sjis);
	std::filesystem::path GetLR2BasePath();
	std::string GetDatabasePath();
	std::string GetConfigPath();
	std::string GetChartPath(std::string hash);
	std::string GetOptionName(unsigned int opt);
	std::string GetGaugeName(unsigned int gauge);
	std::vector<char> LoadFileToVector(std::string path);
	float CalculateRate(network::Score score);
}