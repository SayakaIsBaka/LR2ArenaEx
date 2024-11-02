#pragma once

#include <Garnet/Garnet.h>
#include <array>
#include <memory>
#include <unordered_map>
#include <network/enums.h>
#include <network/structs.h>

namespace server {
	struct State {
		Garnet::Address host;
		std::array<unsigned int, 7> currentRandom = { 0, 0, 0, 0, 0, 0, 0 };
		network::ItemSettings itemSettings;
		bool itemModeEnabled = false;

		std::unordered_map<Garnet::Address, network::Peer> peers;
	};

	inline std::shared_ptr<Garnet::ServerTCP> server;
	inline State state;
	inline bool started = false;
	inline bool autoRotateHost = false;

	bool Start(const char* host = "0.0.0.0", ushort port = 2222);
	bool Stop();

	void Receive(void* data, int size, int actualSize, Garnet::Address clientAddr);
	void ClientConnected(Garnet::Address clientAddr);
	void ClientDisconnected(Garnet::Address clientAddr);
	void SendToEveryone(network::ServerToClient id, std::vector<unsigned char> data, Garnet::Address origSenderAddr, bool includeOrigSender);
	void SendTo(network::ServerToClient id, std::vector<unsigned char> data, Garnet::Address addr);

	void ParsePacket(std::vector<unsigned char> data, Garnet::Address clientAddr);
}