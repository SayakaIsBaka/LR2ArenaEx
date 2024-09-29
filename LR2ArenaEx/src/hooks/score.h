#pragma once

#include <readerwriterqueue/readerwriterqueue.h>

namespace hooks {
	namespace score {
		struct ScoreEvent {
			int poor;
			int bad;
			int good;
			int great;
			int p_great;
			int max_combo;
			int score;
		};

		inline moodycamel::BlockingReaderWriterQueue<ScoreEvent> score_queue(1000);

		void Setup();
	}
}