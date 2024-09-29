#pragma once

#include <string>
#include <enet/enet.h>

namespace client {
	inline ENetHost* client;
	inline ENetPeer* peer = NULL;
	inline HANDLE clientReceiverHandle;
	inline char host[128];
	inline char username[128];
	inline bool connected = false;

	bool Init();
	bool Connect(const char* host);
	bool Destroy();
	void Disconnect();

	DWORD WINAPI ListenLoop(LPVOID lpParam);
	void Send(unsigned char id, std::vector<unsigned char> data);
	void Send(unsigned char id, std::string msg);
	void SendWithRandom(unsigned char id, std::string msg);
}