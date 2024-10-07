#pragma once

namespace network {
	enum class ClientToServer : unsigned char {
		SEND_BMS_PATH = 1,
		SEND_PLAYER_SCORE,
		SEND_CHART_CANCELLED,
		SEND_LOADING_COMPLETE,
	};

	enum class ServerToClient : unsigned char {

	};
}