#pragma once

#include <enet/enet.h>
#include <vector>

namespace server {
	inline ENetHost* server;
	inline std::vector<ENetPeer*> peers;
	inline bool started = false;
	inline HANDLE serverReceiverHandle;

	bool Start();
	DWORD WINAPI ListenLoop(LPVOID lpParam);
	bool Stop();
}