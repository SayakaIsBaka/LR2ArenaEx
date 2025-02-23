#pragma once

#include <framework.h>
#include <array>

namespace hooks {
	namespace random {
		inline int (*GetRandomNumber)(int) = (int(*)(int))0x6C95E0; // rng address
		inline BOOL(__cdecl* ErrorLogFmtAdd)(const char*, ...) = (BOOL(*)(const char*, ...))0x4C8660; // log function address

		inline int current_seed = 0;
		inline bool random_flip = false;
		inline bool received_random = false;

		void Setup();
		void Destroy();
	}
}