#pragma once

#include <string>
#include <Garnet.h>
#include <network/enums.h>

namespace client {
	constexpr int MAX_TCP = 1448;

	inline Garnet::Socket client;
	inline HANDLE clientReceiverHandle;
	inline char host[128];
	inline char username[128];
	inline bool connected = false;

	bool Init();
	bool Connect(const char* host);
	bool Destroy();
	void Disconnect();

	DWORD WINAPI ListenLoop(LPVOID lpParam);
	void Send(network::ClientToServer id, std::vector<unsigned char> data);
	void Send(network::ClientToServer id, std::string msg);
	void SendWithRandom(network::ClientToServer id, std::string msg);
	void ParsePacket(std::vector<unsigned char> data);
}