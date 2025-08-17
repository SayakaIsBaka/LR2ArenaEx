#include <utils/msgpack_utils.h>
#include <iostream>
#include <vector>
#include <hooks/pacemaker.h>
#include <hooks/loadingdone.h>
#include <hooks/random.h>
#include <hooks/maniac.h>
#include <hooks/selectscene.h>
#include <network/structs.h>
#include <utils/misc.h>
#include <gui/mainwindow.h>
#include <filesystem>
#include <ImGui/ImGuiNotify.hpp>
#include <config/config.h>

#include "client.h"

void client::Send(network::ClientToServer id, std::vector<char> data) {
	data.insert(data.begin(), static_cast<char>(id));
	if (client.getReadyState() == ix::ReadyState::Open && connected) {
		auto result = client.sendBinary(data);
		if (result.success == false) // Disconnected, probably timeout or something
			Destroy();
	}
}

void client::Send(network::ClientToServer id, std::string msg) {
	std::vector<char> data;
	data.insert(data.begin(), msg.begin(), msg.end());
	Send(id, data);
}

void client::UnpackPeerList(std::vector<char> data) {
	auto receivedPeers = msgpack_utils::unpack<network::PeerList>(data);
	state.peers = receivedPeers.list;
	state.host = receivedPeers.host;
}

void client::UpdatePeersState(std::vector<char> data) {
	UnpackPeerList(data);

	std::cout << "[+] Connected users:" << std::endl;
	for (const auto& [key, value] : state.peers)
		std::cout << "- " << state.peers[key].username << std::endl;
}

bool client::UpdateReadyState(std::vector<char> data) {
	UnpackPeerList(data);

	bool allReady = true;
	std::string hash = state.peers.begin()->second.selectedHash; // Take first hash as reference, all players must have the same chart selected anyways so no difference
	for (const auto& [key, value] : state.peers) {
		allReady = state.peers[key].ready && state.peers[key].selectedHash == hash;
		if (!allReady) break;
	}
	return allReady;
}

void client::UpdateSelectedSong(std::vector<char> data) {
	auto selectedBms = msgpack_utils::unpack<network::SelectedBmsMessage>(data);
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
		if (!(state.host == state.remoteId))
			hooks::select_scene::SearchSongByHash(selectedBms.hash); // Perform programmatic search if not host
	}
}

unsigned int CalculatePacemakerDisplayScore(std::unordered_map<network::Address, network::Peer> peers) {
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

void client::UpdateScore(std::vector<char> data) {
	auto scoreMsg = msgpack_utils::unpack<network::ScoreMessage>(data);
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

void client::UpdateMessage(std::vector<char> data) {
	auto msg = msgpack_utils::unpack<network::Message>(data);
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

void client::UpdateItem(std::vector<char> data) {
	auto item = msgpack_utils::unpack<network::CurrentItem>(data);
	hooks::maniac::TriggerItem(item);
}

void client::UpdateItemSettings(std::vector<char> data) {
	auto itemSettings = msgpack_utils::unpack<network::ItemSettings>(data);
	hooks::maniac::UpdateItemSettings(itemSettings);
}

void client::ParsePacket(std::vector<char> data) {
	char id = data.front();
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
		state.remoteId = msgpack_utils::unpack<network::Address>(data);
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

void client::OnMessageReceived(const ix::WebSocketMessagePtr& msg) {
	if (msg->type == ix::WebSocketMessageType::Message) {
		std::vector<char> data(msg->str.begin(), msg->str.end());
		ParsePacket(data);
	}
	else if (msg->type == ix::WebSocketMessageType::Open) {
		std::cout << "[+] Connected to " << host << std::endl;
		connected = true;
		hooks::pacemaker::Setup(); // Hook pacemaker

		Send(network::ClientToServer::CTS_USERNAME, std::string(username));

		config::SetConfigValue("username", username);
		config::SetConfigValue("host", host);
		config::SaveConfig();
		ImGui::InsertNotification({ ImGuiToastType::Success, 3000, "Successfully connected to %s", host });
	}
	else if (msg->type == ix::WebSocketMessageType::Close) {
		Destroy();
	}
	else if (msg->type == ix::WebSocketMessageType::Error) {
		std::cout << "[!] Connection to " << host << " failed" << std::endl;
		ImGui::InsertNotification({ ImGuiToastType::Error, 3000, "Connection to %s failed", host });
		Destroy();
	}
	else if (msg->type == ix::WebSocketMessageType::Ping) {
		// Keep-alive received from server, do nothing
	}
	else {
		std::cout << "[!] Unsupported WebSocket message type received: " << (int)(msg->type) << std::endl;
	}
}

void client::Connect(const char* host, const char* username) {
	Disconnect();
	if (connected) { // Reset client if already connected
		Destroy();
	}

	std::string url = "ws://" + std::string(host) + ":2222/";

	client.setUrl(url);
	client.setOnMessageCallback(OnMessageReceived);
	client.disableAutomaticReconnection();
	client.start();
}

void client::Disconnect() {
	client.stop();
}

bool client::Destroy() {
	if (connected == true) {
		connected = false;
		state = ClientState(); // Reset state
		hooks::pacemaker::Destroy(); // Restore original bytes for pacemaker
		ImGui::InsertNotification({ ImGuiToastType::Info, 3000, "Disconnected from the server" });
	}
	return true;
}