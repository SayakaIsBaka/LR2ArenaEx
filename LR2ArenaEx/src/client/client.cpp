#include <msgpack/msgpack.hpp>
#include <iostream>
#include <vector>
#include <hooks/pacemaker.h>
#include <hooks/loadingdone.h>
#include <hooks/random.h>
#include <network/structs.h>
#include <utils/misc.h>

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

void client::UnpackPeerList(std::vector<unsigned char> data) {
	auto receivedPeers = msgpack::unpack<network::PeerList>(data);
	state.peers = receivedPeers.list;
	state.host = receivedPeers.host;
}

void client::UpdatePeersState(std::vector<unsigned char> data) {
	UnpackPeerList(data);

	std::cout << "[+] Connected users:" << std::endl;
	for (const auto& [key, value] : state.peers)
		std::cout << "- " << state.peers[key].username << std::endl;
}

bool client::UpdateReadyState(std::vector<unsigned char> data) {
	UnpackPeerList(data);

	bool allReady = true;
	std::string hash = state.peers.begin()->second.selectedHash; // Take first hash as reference, all players must have the same chart selected anyways so no difference
	for (const auto& [key, value] : state.peers) {
		allReady = state.peers[key].ready && state.peers[key].selectedHash == hash;
		if (!allReady) break;
	}
	return allReady;
}

void client::UpdateSelectedSong(std::vector<unsigned char> data) {
	auto selectedBms = msgpack::unpack<network::SelectedBmsMessage>(data);
	std::cout << "[+] Received selected song" << std::endl;

	// TODO: store hash + song + title somewhere to display

	if (!(state.host == state.remoteId)) { // If not host (!= is not overloaded!!!)
		std::cout << "[+] Received random" << std::endl;
		hooks::random::received_random = true;

		EnterCriticalSection(&hooks::random::RandomCriticalSection);
		hooks::random::current_random = selectedBms.random;
		LeaveCriticalSection(&hooks::random::RandomCriticalSection);

		// TODO: do check on DB if not host
	}
}

unsigned int CalculatePacemakerDisplayScore(std::unordered_map<Garnet::Address, network::Peer> peers) {
	if (peers.size() == 1)
		return utils::CalculateExScore(peers[client::state.remoteId].score); // Return own score if alone in lobby
	unsigned int maxScore = 0;
	for (const auto& [key, value] : peers) {
		if (key == client::state.remoteId) continue; // Skip own score
		unsigned int playerScore = utils::CalculateExScore(value.score);
		maxScore = playerScore > maxScore ? playerScore : maxScore;
	}
	return maxScore;
}

void client::UpdateScore(std::vector<unsigned char> data) {
	auto scoreMsg = msgpack::unpack<network::ScoreMessage>(data);
	state.peers[scoreMsg.player].score = scoreMsg.score;

	hooks::pacemaker::displayed_score = CalculatePacemakerDisplayScore(state.peers);
	if (hooks::pacemaker::pacemaker_address)
		*hooks::pacemaker::pacemaker_address = hooks::pacemaker::displayed_score;
	if (hooks::pacemaker::pacemaker_display_address)
		*hooks::pacemaker::pacemaker_display_address = hooks::pacemaker::displayed_score;
	std::cout << "Max score: " << hooks::pacemaker::displayed_score << std::endl;
}

void client::ParsePacket(std::vector<unsigned char> data) { // TODO: update for multiple players
	unsigned char id = data.front();
	data.erase(data.begin());
	switch ((network::ServerToClient)id)
	{
	case network::ServerToClient::STC_PLAYERS_SCORE:
		UpdateScore(data);
		break;
	case network::ServerToClient::STC_PLAYERS_READY_UPDATE:
		std::cout << "[+] Got updated ready status" << std::endl;
		hooks::loading_done::isEveryoneReady = UpdateReadyState(data);
		break;
	case network::ServerToClient::STC_SELECTED_CHART_RANDOM:
		UpdateSelectedSong(data);
		break;
	case network::ServerToClient::STC_USERLIST:
		UpdatePeersState(data);
		break;
	case network::ServerToClient::STC_CLIENT_REMOTE_ID:
		state.remoteId = msgpack::unpack<Garnet::Address>(data);
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