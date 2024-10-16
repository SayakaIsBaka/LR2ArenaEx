#pragma once

#include <readerwriterqueue/readerwriterqueue.h>
#include <network/structs.h>

namespace hooks {
	namespace score {
		inline moodycamel::BlockingReaderWriterQueue<network::Score> score_queue(1000);

		void Setup();
	}
}