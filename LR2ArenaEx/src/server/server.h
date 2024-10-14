#pragma once

#include <Garnet.h>
#include <unordered_map>
#include <network/structs.h>

namespace server {
	struct State {
		Garnet::Address host;
		unsigned int currentRandom[7] = { 0, 0, 0, 0, 0, 0, 0 };

		std::unordered_map<Garnet::Address, network::Peer> peers;
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