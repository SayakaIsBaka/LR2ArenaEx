#pragma once

#include <network/structs.h>

namespace utils {
	unsigned int CalculateExScore(network::Score score);
	void StrTrim(char* s);
	std::string SJISToUTF8(const std::string& sjis);
	std::string GetDatabasePath();
	bool CheckIfChartExists(std::string hash);
}