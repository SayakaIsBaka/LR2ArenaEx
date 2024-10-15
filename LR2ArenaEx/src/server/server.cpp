#include <iostream>
#include <sstream>
#include <network/enums.h>
#include <msgpack/msgpack.hpp>

#include "server.h"

void ResetState() {
    for (const auto& [key, value] : server::state.peers)
    {
        // Reset state as new chart has been selected
        server::state.peers[key].ready = false;
        server::state.peers[key].selectedHash = "";
    }
}

void ParseSelectedBms(std::vector<unsigned char> data, Garnet::Address clientAddr) {
    auto selectedBms = msgpack::unpack<network::SelectedBmsMessage>(data);

    if (clientAddr == server::state.host) {
        server::state.currentRandom = selectedBms.random;
        ResetState();
    }

    std::cout << "[server] Received selected bms: " << selectedBms.title << " / " << selectedBms.artist << std::endl;
    std::cout << "[server] Hash: " << selectedBms.hash << std::endl;
    std::cout << "[server] Random: " << selectedBms.hash << std::endl;
    for (const auto e : server::state.currentRandom)
        std::cout << e << " ";
    std::cout << std::endl;

    server::state.peers[clientAddr].selectedHash = selectedBms.hash;
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
		break;
	case network::ClientToServer::CTS_CHART_CANCELLED:
        std::cout << "[server] Received chart cancelled" << std::endl;
		break;
	case network::ClientToServer::CTS_LOADING_COMPLETE:
        std::cout << "[server] Received loading complete from " << clientAddr.host << std::endl;
        server::state.peers[clientAddr].ready = true;
        SendToEveryone(network::ServerToClient::STC_PLAYERS_READY_UPDATE, msgpack::pack(network::PeerList(state.peers, state.host)), clientAddr, true);
		break;
    case network::ClientToServer::CTS_USERNAME:
        SetUsername(data, clientAddr);
        SendTo(network::ServerToClient::STC_CLIENT_REMOTE_ID, msgpack::pack(clientAddr), clientAddr); // Send remote address to sender (use as ID)
        SendToEveryone(network::ServerToClient::STC_USERLIST, msgpack::pack(network::PeerList(state.peers, state.host)), clientAddr, true);
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

        auto res = msgpack::pack(network::PeerList(state.peers, state.host));
        res.insert(res.begin(), (unsigned char)network::ServerToClient::STC_USERLIST);

        for (const Garnet::Address& addr : server->getClientAddresses())
        {
            if (clientAddr == addr) continue;
            server->send(&res[0], res.size(), addr);
        }
    }
}

bool server::Start() {
    Garnet::Address addr;
    bool success = false;
    addr.host = "0.0.0.0";
    addr.port = 2222;

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