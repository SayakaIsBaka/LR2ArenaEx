#pragma once

#include <Garnet.h>
#include <vector>

namespace server {
	struct Peer {
		std::string username;
	};

	inline Garnet::ServerTCP* server;
	inline std::vector<Peer> peers;
	inline bool started = false;

	bool Start();
	bool Stop();

	void Receive(void* data, int size, int actualSize, Garnet::Address clientAddr);
	void ClientConnected(Garnet::Address clientAddr);
	void ClientDisconnected(Garnet::Address clientAddr);

	void ParsePacket(std::vector<unsigned char> data);
}