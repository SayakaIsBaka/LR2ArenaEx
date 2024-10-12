#pragma once

namespace network {
	enum class ClientToServer : unsigned char {
		CTS_BMS_PATH = 1,
		CTS_PLAYER_SCORE,
		CTS_CHART_CANCELLED,
		CTS_LOADING_COMPLETE,
	};

	enum class ServerToClient : unsigned char {
		STC_PLAYERS_SCORE = 1,
		STC_PLAYERS_READY_UPDATE,
		STC_RANDOM,
	};
}