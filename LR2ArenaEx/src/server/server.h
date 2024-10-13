#pragma once

#include <Garnet.h>
#include <unordered_map>

namespace server {
	struct Peer {
		std::string username;
		std::string selectedHash;
		bool ready = false;
	};

	struct State {
		Garnet::Address host;
		unsigned int currentRandom[7] = { 0, 0, 0, 0, 0, 0, 0 };

		std::unordered_map<Garnet::Address, Peer> peers;
	};

	inline Garnet::ServerTCP* server;
	inline State state;
	inline bool started = false;

	bool Start();
	bool Stop();

	void Receive(void* data, int size, int actualSize, Garnet::Address clientAddr);
	void ClientConnected(Garnet::Address clientAddr);
	void ClientDisconnected(Garnet::Address clientAddr);

	std::vector<unsigned char> ParsePacket(std::vector<unsigned char> data, Garnet::Address clientAddr);
}