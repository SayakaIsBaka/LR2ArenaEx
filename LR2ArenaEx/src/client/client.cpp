#include <msgpack/msgpack.hpp>
#include <iostream>
#include <vector>
#include <hooks/pacemaker.h>
#include <hooks/loadingdone.h>
#include <hooks/random.h>
#include <hooks/maniac.h>
#include <network/structs.h>
#include <utils/misc.h>
#include <gui/mainwindow.h>
#include <filesystem>
#include <ImGui/ImGuiNotify.hpp>
#include <config/config.h>

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

	state.selectedSongRemote.hash = selectedBms.hash;
	state.selectedSongRemote.artist = selectedBms.artist;
	state.selectedSongRemote.title = selectedBms.title;

	gui::main_window::AddToLog("[#] Selected song: " + selectedBms.title + " / " + selectedBms.artist);
	gui::main_window::AddToLog("[#] Hash: " + selectedBms.hash);

	hooks::maniac::itemModeEnabled = selectedBms.itemModeEnabled;
	if (selectedBms.itemModeEnabled) {
		gui::main_window::AddToLog("[#] Item mode enabled!");
	}

	if (!(state.host == state.remoteId)) { // If not host (!= is not overloaded!!!)
		std::cout << "[+] Received random" << std::endl;
		hooks::random::received_random = true;
		hooks::random::current_seed = selectedBms.randomSeed;
	}

	std::string path = utils::GetChartPath(selectedBms.hash);
	if (path.empty()) {
		state.selectedSongRemote.path = "";
		gui::main_window::AddToLog("[!] You do not have this chart!");
		Send(network::ClientToServer::CTS_MISSING_CHART, "");
	}
	else {
		auto p = std::filesystem::u8path(path);
		std::filesystem::path relativePath = p.parent_path().filename() / p.filename();
		state.selectedSongRemote.path = relativePath.u8string();
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
	if (state.peers.find(scoreMsg.player) == state.peers.end()) {
		std::cout << "[!] Player not found for score update" << std::endl;
		return;
	}
	state.peers[scoreMsg.player].score = scoreMsg.score;

	hooks::pacemaker::displayed_score = CalculatePacemakerDisplayScore(state.peers);
	if (hooks::pacemaker::pacemaker_address)
		*hooks::pacemaker::pacemaker_address = hooks::pacemaker::displayed_score;
	if (hooks::pacemaker::pacemaker_display_address)
		*hooks::pacemaker::pacemaker_display_address = hooks::pacemaker::displayed_score;
	std::cout << "Max score: " << hooks::pacemaker::displayed_score << std::endl;
}

void client::UpdateMessage(std::vector<unsigned char> data) {
	auto msg = msgpack::unpack<network::Message>(data);
	if (msg.systemMessage) {
		gui::main_window::AddToLog(msg.message);
	}
	else {
		if (state.peers.find(msg.player) == state.peers.end()) {
			std::cout << "[!] Player not found for message" << std::endl;
			return;
		}
		gui::main_window::AddToLogWithUser(msg.message, msg.player);
	}
}

void client::UpdateItem(std::vector<unsigned char> data) {
	auto item = msgpack::unpack<network::CurrentItem>(data);
	hooks::maniac::TriggerItem(item);
}

void client::UpdateItemSettings(std::vector<unsigned char> data) {
	auto itemSettings = msgpack::unpack<network::ItemSettings>(data);
	hooks::maniac::UpdateItemSettings(itemSettings);
}

void client::ParsePacket(std::vector<unsigned char> data) {
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
	case network::ServerToClient::STC_MESSAGE:
		UpdateMessage(data);
		break;
	case network::ServerToClient::STC_ITEM:
		UpdateItem(data);
		break;
	case network::ServerToClient::STC_ITEM_SETTINGS:
		UpdateItemSettings(data);
		break;
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
		if (receivedBytes <= 0) { // Probably disconnected or something
			Destroy();
			Init();
			break;
		}
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
		hooks::pacemaker::Setup(); // Hook pacemaker

		DWORD clientReceiverId;
		clientReceiverHandle = CreateThread(NULL, 0, ListenLoop, NULL, 0, &clientReceiverId);
		if (clientReceiverHandle == NULL)
		{
			std::cout << "[!] run::CreateThread clientListenLoop failed" << std::endl;
			return false;
		}
		Send(network::ClientToServer::CTS_USERNAME, std::string(username));

		config::SetConfigValue("username", username);
		config::SetConfigValue("host", host);
		config::SaveConfig();
		ImGui::InsertNotification({ ImGuiToastType::Success, 3000, "Successfully connected to %s", host });

		return true;
	}
	else
	{
		std::cout << "[!] Connection to " << host << " failed" << std::endl;
		ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Connection to %s failed", host });
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
	if (connected == true) {
		connected = false;
		state = ClientState(); // Reset state
		hooks::pacemaker::Destroy(); // Restore original bytes for pacemaker
		ImGui::InsertNotification({ ImGuiToastType::Info, 3000, "Disconnected from the server" });
	}
	return true;
}