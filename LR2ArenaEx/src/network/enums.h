#pragma once

namespace network {
	enum class ClientToServer : unsigned char {
		CTS_SELECTED_BMS = 1,
		CTS_PLAYER_SCORE,
		CTS_CHART_CANCELLED,
		CTS_LOADING_COMPLETE,
		CTS_USERNAME,
		CTS_MESSAGE,
		CTS_MISSING_CHART,
		CTS_SET_HOST,
		CTS_KICK_USER,
		CTS_ITEM,
		CTS_ITEM_SETTINGS,
	};

	enum class ServerToClient : unsigned char {
		STC_PLAYERS_SCORE = 1,
		STC_PLAYERS_READY_UPDATE,
		STC_SELECTED_CHART_RANDOM,
		STC_USERLIST,
		STC_CLIENT_REMOTE_ID,
		STC_MESSAGE,
		STC_MISSING_CHART,
		STC_ITEM,
		STC_ITEM_SETTINGS,
	};

	enum class SelectedOption : unsigned int {
		NONRAN = 0,
		MIRROR,
		RANDOM,
		SRAN,
		HRAN,
		ALLSCR
	};

	enum class SelectedGauge : unsigned int {
		GROOVE = 0,
		HARD,
		HAZARD,
		EASY,
		PATTACK,
		GATTACK
	};
}