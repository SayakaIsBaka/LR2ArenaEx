#pragma once

#include <enet/enet.h>

namespace client {
	inline ENetHost* client;
	inline ENetPeer* peer = NULL;
	inline HANDLE clientReceiverHandle;

	bool Init();
	bool Destroy();

	DWORD WINAPI ListenLoop(LPVOID lpParam);
	void Send(unsigned char id, std::vector<unsigned char> data);
	void Send(unsigned char id, std::string msg);
	void SendWithRandom(unsigned char id, std::string msg);
}