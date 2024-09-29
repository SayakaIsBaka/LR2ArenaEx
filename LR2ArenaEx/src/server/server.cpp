#include <iostream>

#include "server.h"

DWORD WINAPI server::ListenLoop(LPVOID lpParam) {
    ENetEvent event;
    std::vector<unsigned char> data;

    while (true)
    {
        if (!started)
            break; // Stop thread from running if stopping server
        if (enet_host_service(server, &event, 0) > 0) {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                printf("A new client connected from %x:%u.\n",
                    event.peer->address.host,
                    event.peer->address.port);

                event.peer->data = "Client information";

                break;
            case ENET_EVENT_TYPE_RECEIVE:
                printf("A packet of length %u containing %s was received from %s on channel %u.\n",
                    event.packet->dataLength,
                    event.packet->data,
                    event.peer->data,
                    event.channelID);

                data.assign((unsigned char)(event.packet->data), event.packet->dataLength);

                enet_packet_destroy(event.packet);

                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                event.peer->data = NULL;
                printf("Disconnected.\n");
                break;
            }
        }
    }
    return 0;
}

bool server::Start() {
    ENetAddress address;

    address.host = ENET_HOST_ANY;
    address.port = 2222;

    server = enet_host_create(&address /* the address to bind the server host to */,
        8      /* allow up to 8 clients and/or outgoing connections */,
        2      /* allow up to 2 channels to be used, 0 and 1 */,
        0      /* assume any amount of incoming bandwidth */,
        0      /* assume any amount of outgoing bandwidth */);
    if (server == NULL)
    {
        std::cout << "[!] Error creating ENet server" << std::endl;
        return false;
    }
    started = true;

    DWORD serverReceiverId;
    serverReceiverHandle = CreateThread(NULL, 0, ListenLoop, NULL, 0, &serverReceiverId);
    if (serverReceiverHandle == NULL)
    {
        std::cout << "[!] run::CreateThread serverListenLoop failed" << std::endl;
        return false;
    }

    std::cout << "[+] Started server on port " << std::dec << address.port << std::endl;
    return true;
}

bool server::Stop() {
    for (auto p : peers) {
        ENetEvent event;
        bool disconnected = false;

        enet_peer_disconnect(p, 0);
        while (enet_host_service(server, &event, 3000) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(event.packet);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                puts("Disconnection succeeded.");
                disconnected = true;
                break;
            }
        }
        if (!disconnected)
            enet_peer_reset(p);
    }
    started = false;
	enet_host_destroy(server);
    std::cout << "[+] Stopped server" << std::endl;
    return true;
}