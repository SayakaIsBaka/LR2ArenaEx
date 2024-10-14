#pragma once

#include <framework.h>
#include <array>

namespace hooks {
	namespace random {
		inline int (*GetRandomNumber)(int) = (int(*)(int))0x6C95E0; // rng address

		inline std::array<unsigned int, 7> current_random = { 0, 0, 0, 0, 0, 0, 0 };
		inline CRITICAL_SECTION RandomCriticalSection;
		inline bool random_flip = false;
		inline bool received_random = false;

		void Setup();
		void UpdateRandom();
		void Destroy();
	}
}