#include "misc.h"

unsigned int utils::CalculateExScore(network::Score score) {
	return score.p_great * 2 + score.great;
}