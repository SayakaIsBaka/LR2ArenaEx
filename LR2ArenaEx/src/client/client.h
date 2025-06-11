#pragma once

#include <string>
#include <unordered_map>
#include <Garnet/Garnet.h>
#include <network/enums.h>
#include <network/structs.h>

namespace client {
	constexpr int MAX_TCP = 1448;

	struct SelectedSong {
		std::string hash;
		std::string title;
		std::string artist;
		std::string path;
	};

	struct ClientState {
		Garnet::Address remoteId;
		Garnet::Address host;
		SelectedSong selectedSongRemote;
		std::unordered_map<Garnet::Address, network::Peer> peers;
	};

	inline Garnet::Socket client;
	inline HANDLE clientReceiverHandle;
	inline char host[128];
	inline char username[128];
	inline bool connected = false;
	inline ClientState state;

	bool Init();
	bool Connect(const char* host, const char* username);
	bool Destroy();
	void Disconnect();

	DWORD WINAPI ListenLoop(LPVOID lpParam);
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