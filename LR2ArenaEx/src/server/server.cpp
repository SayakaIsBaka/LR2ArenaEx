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
    if (data.size() <= 7 * 4) {
        std::cout << "[!][server] Invalid packet received" << std::endl;
        return;
    }
    if (clientAddr == server::state.host) {
        memcpy(&server::state.currentRandom, &data[0], data.size());
        ResetState();
    }
    data.erase(data.begin(), data.begin() + 7 * 4); // Erase random from the payload, continue with processing

    std::stringstream splitter(std::string(data.begin(), data.end()));
    std::string segment;
    std::vector<std::string> segList;
    while (std::getline(splitter, segment, '\xff'))
        segList.push_back(segment);
    if (segList.size() != 3) {
        std::cout << "[!][server] Invalid packet received" << std::endl;
        return;
    }

    std::cout << "[server] Received selected bms: " << segList[1] << " / " << segList[2] << std::endl;
    std::cout << "[server] Hash: " << segList[0] << std::endl;

    server::state.peers[clientAddr].selectedHash = segList[0];
}

void SetUsername(std::vector<unsigned char> data, Garnet::Address clientAddr) {
    std::string username(data.begin(), data.end());
    std::cout << "[server] Username: " << username << std::endl;
    server::state.peers[clientAddr].username = username;
}

std::vector<unsigned char> server::ParsePacket(std::vector<unsigned char> data, Garnet::Address clientAddr) {
	unsigned char id = data.front();
	data.erase(data.begin());
    std::vector<unsigned char> res;

	switch ((network::ClientToServer)id)
	{
	case network::ClientToServer::CTS_SELECTED_BMS:
        ParseSelectedBms(data, clientAddr);
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
        // Send updated struct to all clients; calculate if should start client-side (useful to show who's ready client-side)
		break;
    case network::ClientToServer::CTS_USERNAME:
        SetUsername(data, clientAddr);
        res = msgpack::pack(network::PeerList(state.peers));
        res.insert(res.begin(), (unsigned char)network::ServerToClient::STC_USERLIST);
        // TODO: send list of users to all clients
        break;
	default:
        std::cout << "[server] Unknown message received" << std::endl;
		break;
	}

    return res;
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
    auto res = ParsePacket(dataVector, clientAddr);

    if (!res.empty()) {
        for (const Garnet::Address& addr : server->getClientAddresses())
        {
            //if (clientAddr == addr) continue;
            server->send(&res[0], res.size(), addr);
        }
    }
    delete data;
}

void server::ClientConnected(Garnet::Address clientAddr)
{
    std::cout << "[server] Client (" + clientAddr.host + ":" + std::to_string(clientAddr.port) + ") connected." << std::endl;

    if (state.peers.size() == 0)
        state.host = clientAddr;
    state.peers[clientAddr] = network::Peer();
}

void server::ClientDisconnected(Garnet::Address clientAddr)
{
    std::cout << "[server] Client (" + clientAddr.host + ":" + std::to_string(clientAddr.port) + ") disconnected." << std::endl;
    state.peers.erase(clientAddr);
    if (clientAddr == state.host && state.peers.size() > 0)
        state.host = state.peers.begin()->first; // Change host to first peer in the list

    auto res = msgpack::pack(network::PeerList(state.peers));
    res.insert(res.begin(), (unsigned char)network::ServerToClient::STC_USERLIST);

    for (const Garnet::Address& addr : server->getClientAddresses())
    {
        if (clientAddr == addr) continue;
        server->send(&res[0], res.size(), addr);
    }
}

bool server::Start() {
    Garnet::Address addr;
    bool success = false;
    addr.host = "0.0.0.0";
    addr.port = 2222;

    server = new Garnet::ServerTCP(addr, &success);

    if (!success)
    {
        std::cout << "[!] Error creating ENet server" << std::endl;
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
    server->close();
    started = false;
	delete server;
    std::cout << "[+] Stopped server" << std::endl;
    return true;
}