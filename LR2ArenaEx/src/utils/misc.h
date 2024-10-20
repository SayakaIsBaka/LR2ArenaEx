#pragma once

#include <network/structs.h>

namespace utils {
	unsigned int CalculateExScore(network::Score score);
	void StrTrim(char* s);
	std::string SJISToUTF8(const std::string& sjis);
	std::string GetDatabasePath();
	std::string GetChartPath(std::string hash);
	std::string GetOptionName(unsigned int opt);
}