#pragma once

namespace network {
	enum class ClientToServer : unsigned char {
		CTS_SELECTED_BMS = 1,
		CTS_PLAYER_SCORE,
		CTS_CHART_CANCELLED,
		CTS_LOADING_COMPLETE,
		CTS_USERNAME,
	};

	enum class ServerToClient : unsigned char {
		STC_PLAYERS_SCORE = 1,
		STC_PLAYERS_READY_UPDATE,
		STC_RANDOM,
	};
}