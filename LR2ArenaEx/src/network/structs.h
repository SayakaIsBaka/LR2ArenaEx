#pragma once

#include <string>
#include <array>
#include <unordered_map>
#include <Garnet/Garnet.h>

namespace network {
	struct Score {
		int poor;
		int bad;
		int good;
		int great;
		int p_great;
		int max_combo;
		int score;

		template<class T>
		void pack(T& pack) {
			pack(poor, bad, good, great, p_great, max_combo, score);
		}
	};

	struct Peer {
		std::string username;
		std::string selectedHash;
		bool ready = false;
		Score score = Score();
		unsigned int option;
		unsigned int gauge;

		template<class T>
		void pack(T& pack) {
			pack(username, selectedHash, ready, score, option, gauge);
		}
	};

	struct PeerList { // Only used for networking
		std::unordered_map<Garnet::Address, network::Peer> list;
		Garnet::Address host;

		PeerList() {};

		PeerList(std::unordered_map<Garnet::Address, network::Peer> list, Garnet::Address host) {
			this->list = list;
			this->host = host;
		}

		template<class T>
		void pack(T& pack) {
			pack(list, host);
		}
	};

	struct SelectedBmsMessage {
		std::array<unsigned int, 7> random = { 0, 0, 0, 0, 0, 0, 0 };
		std::string hash;
		std::string title;
		std::string artist;
		unsigned int option;
		unsigned int gauge;
		bool itemModeEnabled;

		template<class T>
		void pack(T& pack) {
			pack(random, hash, title, artist, option, gauge, itemModeEnabled);
		}
	};

	struct ScoreMessage { // Used from server to clients
		Score score = Score();
		Garnet::Address player;

		ScoreMessage() {};

		ScoreMessage(Score score, Garnet::Address player) {
			this->score = score;
			this->player = player;
		}

		template<class T>
		void pack(T& pack) {
			pack(score, player);
		}
	};

	struct Message { // Used from server to clients
		std::string message;
		Garnet::Address player;
		bool systemMessage;

		Message() {};

		Message(std::string message, Garnet::Address player, bool systemMessage) {
			this->message = message;
			this->player = player;
			this->systemMessage = systemMessage;
		}

		template<class T>
		void pack(T& pack) {
			pack(message, player, systemMessage);
		}
	};

	struct CurrentItem {
		int rolledItemId = -1;
		unsigned short level = 0;

		bool operator==(const CurrentItem& other) const {
			return (rolledItemId == other.rolledItemId && level == other.level);
		}

		template<class T>
		void pack(T& pack) {
			pack(rolledItemId, level);
		}
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