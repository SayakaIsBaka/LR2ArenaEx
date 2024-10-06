#include <iostream>

#include "server.h"

void server::Receive(void* data, int size, int actualSize, Garnet::Address clientAddr)
{
    std::string msg = "Client (" + clientAddr.host + ":" + std::to_string(clientAddr.port) + "): " + std::string((char*)data, actualSize);
    std::cout << msg << "\n";
    for (const Garnet::Address& addr : server->getClientAddresses())
    {
        if (clientAddr == addr) continue;
        server->send((void*)msg.c_str(), strlen(msg.c_str()), addr);
    }
    delete data;
}

void server::ClientConnected(Garnet::Address clientAddr)
{
    std::string msg = "Client (" + clientAddr.host + ":" + std::to_string(clientAddr.port) + ") connected.";
    std::cout << msg << "\n";
    for (const Garnet::Address& addr : server->getClientAddresses())
    {
        if (clientAddr == addr) continue;
        server->send((void*)msg.c_str(), strlen(msg.c_str()), addr);
    }
}

void server::ClientDisconnected(Garnet::Address clientAddr)
{
    std::string msg = "Client (" + clientAddr.host + ":" + std::to_string(clientAddr.port) + ") disconnected.";
    std::cout << msg << "\n";
    for (const Garnet::Address& addr : server->getClientAddresses())
    {
        if (clientAddr == addr) continue;
        server->send((void*)msg.c_str(), strlen(msg.c_str()), addr);
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