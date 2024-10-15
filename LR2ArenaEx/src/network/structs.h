#pragma once

#include <string>
#include <array>
#include <unordered_map>
#include <Garnet.h>

namespace network {
	struct Peer {
		std::string username;
		std::string selectedHash;
		bool ready = false;

		template<class T>
		void pack(T& pack) {
			pack(username, selectedHash, ready);
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

		template<class T>
		void pack(T& pack) {
			pack(random, hash, title, artist);
		}
	};
}