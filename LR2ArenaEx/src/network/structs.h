#pragma once

#include <msgpack.hpp>
#include <string>
#include <array>
#include <unordered_map>

namespace network {
	struct Address {
		std::string host = "";
		unsigned short port = 0;

		void operator=(const Address& other) {
			host = other.host;
			port = other.port;
		};
		bool operator==(const Address& other) const {
			return (host == other.host && port == other.port);
		};

		MSGPACK_DEFINE(host, port);
	};
}

template <>
struct std::hash<network::Address>
{
	size_t operator()(const network::Address& addr) const
	{
		return hash<string>()(addr.host) ^ (hash<int>()(addr.port) << 1);
	}
};

namespace network {
	struct Score {
		int poor;
		int bad;
		int good;
		int great;
		int p_great;
		int max_combo;
		int score;

		MSGPACK_DEFINE(poor, bad, good, great, p_great, max_combo, score);
	};

	struct Peer {
		std::string username;
		std::string selectedHash;
		bool ready = false;
		Score score = Score();
		unsigned int option;
		unsigned int gauge;

		MSGPACK_DEFINE(username, selectedHash, ready, score, option, gauge);
	};

	struct PeerList { // Only used for networking
		std::unordered_map<network::Address, network::Peer> list;
		network::Address host;

		PeerList() {};

		PeerList(std::unordered_map<network::Address, network::Peer> list, network::Address host) {
			this->list = list;
			this->host = host;
		}

		MSGPACK_DEFINE(list, host);
	};

	struct SelectedBmsMessage {
		int randomSeed;
		std::string hash;
		std::string title;
		std::string artist;
		unsigned int option;
		unsigned int gauge;
		bool itemModeEnabled;

		MSGPACK_DEFINE(randomSeed, hash, title, artist, option, gauge, itemModeEnabled);
	};

	struct ScoreMessage { // Used from server to clients
		Score score = Score();
		network::Address player;

		ScoreMessage() {};

		ScoreMessage(Score score, network::Address player) {
			this->score = score;
			this->player = player;
		}

		MSGPACK_DEFINE(score, player);
	};

	struct Message { // Used from server to clients
		std::string message;
		network::Address player;
		bool systemMessage;

		Message() {};

		Message(std::string message, network::Address player, bool systemMessage) {
			this->message = message;
			this->player = player;
			this->systemMessage = systemMessage;
		}

		MSGPACK_DEFINE(message, player, systemMessage);
	};

	struct CurrentItem {
		int rolledItemId = -1;
		unsigned short level = 0;
		bool final = false;

		bool operator==(const CurrentItem& other) const {
			return (rolledItemId == other.rolledItemId && level == other.level);
		}

		MSGPACK_DEFINE(rolledItemId, level);
	};

	struct ItemSetting {
		unsigned int lv1;
		unsigned int lv2;
		unsigned int lv3;
		unsigned int weight;

		MSGPACK_DEFINE(lv1, lv2, lv3, weight);
	};

	struct ItemSettings {
		unsigned int threshold;
		std::vector<ItemSetting> settings;

		MSGPACK_DEFINE(threshold, settings);
	};
}

template <>
struct std::hash<network::CurrentItem>
{
	std::size_t operator()(const network::CurrentItem& k) const
	{
		using std::size_t;
		using std::hash;

		return (hash<int>()(k.rolledItemId)
			^ (hash<int>()(k.level) << 1));
	}
};