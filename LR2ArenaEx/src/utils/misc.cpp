#include "misc.h"

unsigned int utils::CalculateExScore(network::Score score) {
	return score.p_great * 2 + score.great;
}

// From imgui_demo.cpp
void utils::StrTrim(char* s) {
	char* str_end = s + strlen(s);
	while (str_end > s && isspace(str_end[-1]))
		str_end--;
	*str_end = 0;
}