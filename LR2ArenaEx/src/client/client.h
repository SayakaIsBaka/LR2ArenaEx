#pragma once

#include <string>
#include <unordered_map>
#include <ixwebsocket/IXWebSocket.h>
#include <network/enums.h>
#include <network/structs.h>

namespace client {
	struct SelectedSong {
		std::string hash;
		std::string title;
		std::string artist;
		std::string path;
	};

	struct ClientState {
		network::Address remoteId;
		network::Address host;
		SelectedSong selectedSongRemote;
		std::unordered_map<network::Address, network::Peer> peers;
	};

	inline ix::WebSocket client;
	inline char host[128];
	inline char username[128];
	inline bool connected = false;
	inline ClientState state;

	void Connect(const char* host, const char* username);
	bool Destroy();
	void Disconnect();

	void OnMessageReceived(const ix::WebSocketMessagePtr& msg);
	void Send(network::ClientToServer id, std::vector<char> data);
	void Send(network::ClientToServer id, std::string msg);
	void ParsePacket(std::vector<char> data);
	void UnpackPeerList(std::vector<char> data);

	void UpdatePeersState(std::vector<char> data);
	bool UpdateReadyState(std::vector<char> data);
	void UpdateSelectedSong(std::vector<char> data);
	void UpdateScore(std::vector<char> data);
	void UpdateMessage(std::vector<char> data);
	void UpdateItem(std::vector<char> data);
	void UpdateItemSettings(std::vector<char> data);
}