#pragma once

#include <network/structs.h>

namespace utils {
	unsigned int CalculateExScore(network::Score score);
	void StrTrim(char* s);
	std::string SJISToUTF8(const std::string& sjis);
	std::string GetDatabasePath();
	std::string GetConfigPath();
	std::string GetChartPath(std::string hash);
	std::string GetOptionName(unsigned int opt);
	std::string GetGaugeName(unsigned int gauge);
	float CalculateRate(network::Score score, unsigned int maxScore);
}