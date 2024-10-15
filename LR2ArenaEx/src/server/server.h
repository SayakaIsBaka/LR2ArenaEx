#pragma once

#include <Garnet.h>
#include <array>
#include <unordered_map>
#include <network/enums.h>
#include <network/structs.h>

namespace server {
	struct State {
		Garnet::Address host;
		std::array<unsigned int, 7> currentRandom = { 0, 0, 0, 0, 0, 0, 0 };

		std::unordered_map<Garnet::Address, network::Peer> peers;
	};

	inline std::shared_ptr<Garnet::ServerTCP> server;
	inline State state;
	inline bool started = false;

	bool Start();
	bool Stop();

	void Receive(void* data, int size, int actualSize, Garnet::Address clientAddr);
	void ClientConnected(Garnet::Address clientAddr);
	void ClientDisconnected(Garnet::Address clientAddr);
	void SendToEveryone(network::ServerToClient id, std::vector<unsigned char> data, Garnet::Address origSenderAddr, bool includeOrigSender);

	void ParsePacket(std::vector<unsigned char> data, Garnet::Address clientAddr);
}