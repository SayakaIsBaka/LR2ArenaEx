#include <iostream>
#include <sstream>
#include <network/enums.h>
#include <msgpack/msgpack.hpp>

#include "server.h"

void ResetState(Garnet::Address player) {
    server::state.peers[player].ready = false;
    server::state.peers[player].selectedHash = "";
    server::state.peers[player].score = network::Score();
}

void ResetStateEveryone() {
    for (const auto& [key, value] : server::state.peers)
    {
        // Reset state as new chart has been selected
        ResetState(key);
    }
}

void ParseSelectedBms(std::vector<unsigned char> data, Garnet::Address clientAddr) {
    auto selectedBms = msgpack::unpack<network::SelectedBmsMessage>(data);

    if (clientAddr == server::state.host) {
        server::state.currentRandomSeed = selectedBms.randomSeed;
        server::state.itemModeEnabled = selectedBms.itemModeEnabled;
        ResetStateEveryone();
    }

    std::cout << "[server] Received selected bms: " << selectedBms.title << " / " << selectedBms.artist << std::endl;
    std::cout << "[server] Hash: " << selectedBms.hash << std::endl;
    std::cout << "[server] Random seed: " << server::state.currentRandomSeed << std::endl;

    server::state.peers[clientAddr].selectedHash = selectedBms.hash;
    server::state.peers[clientAddr].option = selectedBms.option;
    server::state.peers[clientAddr].gauge = selectedBms.gauge;
}

void ParseScore(std::vector<unsigned char> data, Garnet::Address clientAddr) {
    auto score = msgpack::unpack<network::Score>(data);
    server::state.peers[clientAddr].score = score;
}

void SetUsername(std::vector<unsigned char> data, Garnet::Address clientAddr) { // Performing state update here to avoid race condition
    std::string username(data.begin(), data.end());
    std::cout << "[server] Username: " << username << std::endl;
    if (server::state.peers.size() == 0) { // If first user to connect, set as host
        server::state.host = clientAddr;
    }
    server::state.peers[clientAddr] = network::Peer();
    server::state.peers[clientAddr].username = username;
}

void SetHost(std::vector<unsigned char> data) {
    auto newHost = msgpack::unpack<Garnet::Address>(data);
    if (server::state.peers.find(newHost) == server::state.peers.end()) {
        std::cout << "[!][server] Player not found for new host" << std::endl;
        return;
    }
    server::state.host = newHost;
}

bool IsEveryoneReady() {
    bool allReady = true;
    std::string hash = server::state.peers.begin()->second.selectedHash; // Take first hash as reference, all players must have the same chart selected anyways so no difference
    for (const auto& [key, value] : server::state.peers) {
        allReady = server::state.peers[key].ready && server::state.peers[key].selectedHash == hash;
        if (!allReady) break;
    }
    return allReady;
}

void AutoRotateHost() {
    auto currentHost = server::state.host;
    bool isNext = false;
    Garnet::Address first;
    for (const auto [key, value] : server::state.peers) {
        if (first.host.empty())
            first = key;
        if (isNext) {
            isNext = false;
            server::state.host = key;
            break;
        }
        if (key == currentHost)
            isNext = true;
    }
    if (isNext)
        server::state.host = first;
}

void SetItemSettings(std::vector<unsigned char> data) {
    auto itemSettings = msgpack::unpack<network::ItemSettings>(data);
    server::state.itemSettings = itemSettings;
}

void server::SendToEveryone(network::ServerToClient id, std::vector<unsigned char> data, Garnet::Address origSenderAddr, bool includeOrigSender) {
    data.insert(data.begin(), static_cast<unsigned char>(id));
    for (const Garnet::Address& addr : server->getClientAddresses())
    {
       if (origSenderAddr == addr && !includeOrigSender)
           continue;
        server->send(&data[0], data.size(), addr);
    }
}

void server::SendTo(network::ServerToClient id, std::vector<unsigned char> data, Garnet::Address addr) {
    data.insert(data.begin(), static_cast<unsigned char>(id));
    server->send(&data[0], data.size(), addr);
}

void server::ParsePacket(std::vector<unsigned char> data, Garnet::Address clientAddr) {
	unsigned char id = data.front();
	data.erase(data.begin());

	switch ((network::ClientToServer)id)
	{
	case network::ClientToServer::CTS_SELECTED_BMS:
        ParseSelectedBms(data, clientAddr);
        if (clientAddr == state.host)
            SendToEveryone(network::ServerToClient::STC_SELECTED_CHART_RANDOM, data, clientAddr, true);
        // If not host, send random + hash + received BMS to other clients; otherwise do nothing?
		break;
	case network::ClientToServer::CTS_PLAYER_SCORE:
        std::cout << "[server] Received player score" << std::endl;
        ParseScore(data, clientAddr);
        SendToEveryone(network::ServerToClient::STC_PLAYERS_SCORE, msgpack::pack(network::ScoreMessage(state.peers[clientAddr].score, clientAddr)), clientAddr, true);
		break;
	case network::ClientToServer::CTS_CHART_CANCELLED:
        std::cout << "[server] Received chart cancelled" << std::endl;
        server::state.peers[clientAddr].ready = false;
        server::state.peers[clientAddr].selectedHash = "";
        SendToEveryone(network::ServerToClient::STC_PLAYERS_READY_UPDATE, msgpack::pack(network::PeerList(state.peers, state.host)), clientAddr, true);
		break;
	case network::ClientToServer::CTS_LOADING_COMPLETE:
        std::cout << "[server] Received loading complete from " << clientAddr.host << std::endl;
        server::state.peers[clientAddr].ready = true;
        if (autoRotateHost && IsEveryoneReady())
            AutoRotateHost();
        SendToEveryone(network::ServerToClient::STC_PLAYERS_READY_UPDATE, msgpack::pack(network::PeerList(state.peers, state.host)), clientAddr, true);
		break;
    case network::ClientToServer::CTS_USERNAME:
        SetUsername(data, clientAddr);
        SendTo(network::ServerToClient::STC_CLIENT_REMOTE_ID, msgpack::pack(clientAddr), clientAddr); // Send remote address to sender (use as ID)
        if (!server::state.itemSettings.settings.empty())
            SendTo(network::ServerToClient::STC_ITEM_SETTINGS, msgpack::pack(server::state.itemSettings), clientAddr); // Send custom item settings if defined
        SendToEveryone(network::ServerToClient::STC_USERLIST, msgpack::pack(network::PeerList(state.peers, state.host)), clientAddr, true);
        break;
    case network::ClientToServer::CTS_MESSAGE:
        std::cout << "[server] Received message" << std::endl;
        SendToEveryone(network::ServerToClient::STC_MESSAGE, msgpack::pack(network::Message(std::string(data.begin(), data.end()), clientAddr, false)), clientAddr, false);
        break;
    case network::ClientToServer::CTS_MISSING_CHART:
        std::cout << "[server] Received missing chart" << std::endl;
        SendToEveryone(network::ServerToClient::STC_MESSAGE, msgpack::pack(network::Message("[!] " + state.peers[clientAddr].username + " is missing the selected chart!", clientAddr, true)), clientAddr, false);
        break;
    case network::ClientToServer::CTS_SET_HOST:
        std::cout << "[server] Received set host" << std::endl;
        if (state.host == clientAddr) {
            SetHost(data);
            SendToEveryone(network::ServerToClient::STC_USERLIST, msgpack::pack(network::PeerList(state.peers, state.host)), clientAddr, true);
        }
        else {
            std::cout << "[!][server] Sender is not the host!" << std::endl;
        }
        break;
    case network::ClientToServer::CTS_KICK_USER:
        std::cout << "[server] Received kick user" << std::endl;
        if (state.host == clientAddr) {
            server->getClientAcceptedSocket(msgpack::unpack<Garnet::Address>(data)).close();
        }
        else {
            std::cout << "[!][server] Sender is not the host!" << std::endl;
        }
        break;
    case network::ClientToServer::CTS_ITEM:
        std::cout << "[server] Received item use" << std::endl;
        SendToEveryone(network::ServerToClient::STC_ITEM, data, clientAddr, false);
        break;
    case network::ClientToServer::CTS_ITEM_SETTINGS:
        std::cout << "[server] Received item settings" << std::endl;
        if (state.host == clientAddr) {
            SetItemSettings(data);
            SendToEveryone(network::ServerToClient::STC_ITEM_SETTINGS, data, clientAddr, false);
        } else {
            std::cout << "[!][server] Sender is not the host!" << std::endl;
        }
        break;
	default:
        std::cout << "[server] Unknown message received" << std::endl;
		break;
	}
}

void server::Receive(void* data, int bufferSize, int actualSize, Garnet::Address clientAddr)
{
    if (actualSize == 0)
        return;
    if (actualSize > bufferSize) {
        std::cout << "[!!!][server] actualSize higher than bufferSize (this shouldn't happen)" << std::endl;
        delete data;
        return;
    }
    std::vector<unsigned char> dataVector((unsigned char*)data, (unsigned char*)data + actualSize);
    ParsePacket(dataVector, clientAddr);

    delete data;
}

void server::ClientConnected(Garnet::Address clientAddr)
{
    std::cout << "[server] Client (" + clientAddr.host + ":" + std::to_string(clientAddr.port) + ") connected." << std::endl;
}

void server::ClientDisconnected(Garnet::Address clientAddr)
{
    std::cout << "[server] Client (" + clientAddr.host + ":" + std::to_string(clientAddr.port) + ") disconnected." << std::endl;
    if (started) {
        state.peers.erase(clientAddr);
        if (clientAddr == state.host && state.peers.size() > 0)
            state.host = state.peers.begin()->first; // Change host to first peer in the list

        if (state.peers.size() > 0) {
            auto res = msgpack::pack(network::PeerList(state.peers, state.host));
            res.insert(res.begin(), (unsigned char)network::ServerToClient::STC_USERLIST);
        
            for (const Garnet::Address& addr : server->getClientAddresses())
            {
                if (clientAddr == addr) continue;
                server->send(&res[0], res.size(), addr);
            }
        }
    }
}

bool server::Start(const char* host, ushort port) {
    Garnet::Address addr;
    bool success = false;
    addr.host = host;
    addr.port = port;

    // Hopefully this is enough to avoid a memleak because delete on a regular pointer crashes the thing
    server = std::make_shared<Garnet::ServerTCP>(addr, &success);

    if (!success)
    {
        std::cout << "[!] Error creating server" << std::endl;
        return false;
    }
    started = true;

    server->setReceiveCallback(Receive);
    server->setClientConnectCallback(ClientConnected);
    server->setClientDisconnectCallback(ClientDisconnected);
    server->open();

    std::cout << "[+] Started server on port " << std::dec << addr.port << std::endl;

    return true;
}

bool server::Stop() {
    bool success = false;
    server->close(&success);
    if (!success) {
        std::cout << "[!] Error stopping server" << std::endl;
        return false;
    }
    started = false;
    state = State();
    std::cout << "[+] Stopped server" << std::endl;
    return true;
}