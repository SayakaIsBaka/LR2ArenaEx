#pragma once

namespace hooks {
	namespace select_bms {
		constexpr uintptr_t selectedGaugeAddr = 0xFF840;

		void Setup();
	}
}