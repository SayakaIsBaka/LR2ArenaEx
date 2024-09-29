#pragma once

namespace hooks {
	namespace pacemaker {
		inline unsigned int* pacemaker_address = NULL;
		inline unsigned int* pacemaker_display_address = NULL;
		inline unsigned int p2_score = 0;

		void Setup();
	}
}