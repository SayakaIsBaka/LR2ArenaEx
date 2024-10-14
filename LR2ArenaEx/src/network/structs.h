#pragma once

#include <string>
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

		PeerList() {};

		PeerList(std::unordered_map<Garnet::Address, network::Peer> list) {
			this->list = list;
		}

		template<class T>
		void pack(T& pack) {
			pack(list);
		}
	};
}