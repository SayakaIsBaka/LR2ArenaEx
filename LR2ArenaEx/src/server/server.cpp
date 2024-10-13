#include <iostream>
#include <network/enums.h>

#include "server.h"

void server::ParsePacket(std::vector<unsigned char> data, Garnet::Address clientAddr) {
	unsigned char id = data.front();
	data.erase(data.begin());
	switch ((network::ClientToServer)id)
	{
	case network::ClientToServer::CTS_SELECTED_BMS:
        std::cout << "[server] received selected bms" << std::endl;
		break;
	case network::ClientToServer::CTS_PLAYER_SCORE:
        std::cout << "[server] received player score" << std::endl;
		break;
	case network::ClientToServer::CTS_CHART_CANCELLED:
        std::cout << "[server] received chart cancelled" << std::endl;
		break;
	case network::ClientToServer::CTS_LOADING_COMPLETE:
        std::cout << "[server] received loading complete" << std::endl;
		break;
    case network::ClientToServer::CTS_USERNAME:
        std::cout << "[server] Username: " << std::string(data.begin(), data.end()) << std::endl;
        state.peers[clientAddr].username = std::string(data.begin(), data.end());
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

    for (const Garnet::Address& addr : server->getClientAddresses())
    {
        if (clientAddr == addr) continue;
        //server->send((void*)msg.c_str(), strlen(msg.c_str()), addr);
    }
    delete data;
}

void server::ClientConnected(Garnet::Address clientAddr)
{
    std::cout << "[server] Client (" + clientAddr.host + ":" + std::to_string(clientAddr.port) + ") connected." << std::endl;

    if (state.peers.size() == 0)
        state.host = clientAddr;
    state.peers[clientAddr] = Peer();

    /*
    for (const Garnet::Address& addr : server->getClientAddresses())
    {
        if (clientAddr == addr) continue;
        server->send((void*)msg.c_str(), strlen(msg.c_str()), addr);
    }
    */
}

void server::ClientDisconnected(Garnet::Address clientAddr)
{
    std::cout << "[server] Client (" + clientAddr.host + ":" + std::to_string(clientAddr.port) + ") disconnected." << std::endl;
    state.peers.erase(clientAddr);
    if (clientAddr == state.host && state.peers.size() > 0)
        state.host = state.peers.begin()->first; // Change host to first peer in the list

    /*
    for (const Garnet::Address& addr : server->getClientAddresses())
    {
        if (clientAddr == addr) continue;
        server->send((void*)msg.c_str(), strlen(msg.c_str()), addr);
    }
    */
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