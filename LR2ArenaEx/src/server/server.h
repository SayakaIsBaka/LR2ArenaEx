#pragma once

#include <ixwebsocket/IXWebSocketServer.h>
#include <array>
#include <memory>
#include <unordered_map>
#include <network/enums.h>
#include <network/structs.h>

namespace server {
	constexpr int MAX_TCP = 1448;

	struct State {
		network::Address host;
		int currentRandomSeed;
		network::ItemSettings itemSettings;
		bool itemModeEnabled = false;

		std::unordered_map<network::Address, network::Peer> peers;
	};

	inline std::shared_ptr<ix::WebSocketServer> server;
	inline State state;
	inline bool started = false;
	inline bool autoRotateHost = false;

	bool Start(const char* host = "0.0.0.0", unsigned short port = 2222);
	bool Stop();

	void OnClientMessageReceived(std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr& msg);
	void ClientConnected(network::Address clientAddr);
	void ClientDisconnected(network::Address clientAddr);
	void SendToEveryone(network::ServerToClient id, std::vector<char> data, network::Address origSenderAddr, bool includeOrigSender);
	void SendTo(network::ServerToClient id, std::vector<char> data, network::Address addr);

	void ParsePacket(std::vector<char> data, network::Address clientAddr);
}