#include <msgpack/msgpack.hpp>
#include <iostream>
#include <vector>
#include <hooks/pacemaker.h>
#include <hooks/loadingdone.h>
#include <hooks/random.h>
#include <network/structs.h>

#include "client.h"

void client::Send(network::ClientToServer id, std::vector<unsigned char> data) {
	data.insert(data.begin(), static_cast<unsigned char>(id));
	int buffer_length = data.size();

	if (client.isOpen() && connected) {
		if (buffer_length > MAX_TCP) {
			std::cout << "[!] Packet size is over the defined limit (" << MAX_TCP << "), this shouldn't happen" << std::endl;
			buffer_length = MAX_TCP;
		}
		client.send(&data[0], data.size());
	}
}

void client::Send(network::ClientToServer id, std::string msg) {
	std::vector<unsigned char> data;
	data.insert(data.begin(), msg.begin(), msg.end());
	Send(id, data);
}

void client::UpdatePeersState(std::vector<unsigned char> data) {
	auto receivedPeers = msgpack::unpack<network::PeerList>(data);
	peers = receivedPeers.list;
	std::cout << "[+] Connected users:" << std::endl;
	for (const auto& [key, value] : peers)
		std::cout << "- " << peers[key].username << std::endl;
}

bool client::UpdateReadyState(std::vector<unsigned char> data) {
	auto receivedPeers = msgpack::unpack<network::PeerList>(data);
	peers = receivedPeers.list;
	bool allReady = true;
	std::string hash = peers.begin()->second.selectedHash; // Take first hash as reference, all players must have the same chart selected anyways so no difference
	for (const auto& [key, value] : peers) {
		allReady = peers[key].ready && peers[key].selectedHash == hash;
		if (!allReady) break;
	}
	return allReady;
}

void client::ParsePacket(std::vector<unsigned char> data) { // TODO: update for multiple players
	unsigned char id = data.front();
	data.erase(data.begin());
	switch ((network::ServerToClient)id)
	{
	case network::ServerToClient::STC_PLAYERS_SCORE: // TODO: update
		if (data.size() <= 0 || data.size() > sizeof(unsigned int)) {
			break;
		}
		fprintf(stdout, "datasize : %u\n", data.size());
		hooks::pacemaker::p2_score = 0; // TODO: Should instead display highest score from all players (or 2nd if you're currently 1st)
		memcpy(&hooks::pacemaker::p2_score, &data[0], data.size()); // little-endian, kinda ugly but it works(tm)
		if (hooks::pacemaker::pacemaker_address)
			*hooks::pacemaker::pacemaker_address = hooks::pacemaker::p2_score;
		if (hooks::pacemaker::pacemaker_display_address)
			*hooks::pacemaker::pacemaker_display_address = hooks::pacemaker::p2_score;
		fprintf(stdout, "p2score : %u\n", hooks::pacemaker::p2_score);
		break;
	case network::ServerToClient::STC_PLAYERS_READY_UPDATE:
		std::cout << "[+] Got updated ready status" << std::endl;
		if (UpdateReadyState(data))
			hooks::loading_done::isEveryoneReady = true;
		break;
	case network::ServerToClient::STC_RANDOM: // TODO: update
		if (data.size() != 7 * 4) {
			fprintf(stderr, "invalid size random\n");
			break;
		}
		fprintf(stdout, "received random\n");
		hooks::random::received_random = true;
		EnterCriticalSection(&hooks::random::RandomCriticalSection);
		memcpy(&hooks::random::current_random[0], &data[0], data.size());
		LeaveCriticalSection(&hooks::random::RandomCriticalSection);
		break;
	case network::ServerToClient::STC_USERLIST:
		UpdatePeersState(data);
		break;
		/*
	case 4: // no need for a random flip packet type anymore as UI is directly integrated
		hooks::random::random_flip = data.front() == 1;
		if (hooks::random::random_flip) {
			fprintf(stdout, "random flip enabled\n");
		}
		else {
			fprintf(stdout, "random flip disabled\n");
		}
		break;
	case 5:
		fprintf(stdout, "P2 does not have the selected chart, please go back to the main menu!\n");
		break;
	case 6:
		fprintf(stdout, "message received\n");
		break;
		*/
	default:
		break;
	}
}

DWORD WINAPI client::ListenLoop(LPVOID lpParam) {
	std::vector<unsigned char> data(MAX_TCP);

    while (true)
    {
		data.resize(MAX_TCP);
		if (!connected)
			break;
		auto receivedBytes = client.receive(&data[0], MAX_TCP);
		if (receivedBytes <= 0) // Probably disconnected or something
			break;
		data.resize(receivedBytes);
		ParsePacket(data);
    }

	return 0;
}

bool client::Connect(const char* host, const char* username) {

	if (connected) { // Reset client if already connected
		Destroy();
		Init();
	}

	Garnet::Address addr;
	bool success = false;
	addr.host = host;
	addr.port = 2222;

	client.connect(addr, &success);

	if (success)
	{
		std::cout << "[+] Connected to " << host << std::endl;
		connected = true;

		DWORD clientReceiverId;
		clientReceiverHandle = CreateThread(NULL, 0, ListenLoop, NULL, 0, &clientReceiverId);
		if (clientReceiverHandle == NULL)
		{
			std::cout << "[!] run::CreateThread clientListenLoop failed" << std::endl;
			return false;
		}
		Send(network::ClientToServer::CTS_USERNAME, std::string(username));
		return true;
	}
	else
	{
		std::cout << "[!] Connection to " << host << " failed" << std::endl;
		return false;
	}
}

bool client::Init() {
	bool success = false;
	client = Garnet::Socket(Garnet::Protocol::TCP, &success);
	if (!success)
	{
		std::cout << "[!] Error creating TCP client" << std::endl;
	}
	return success;
}

void client::Disconnect() {
	client.close();
}

bool client::Destroy() {
	Disconnect();
	connected = false;
	return true;
}