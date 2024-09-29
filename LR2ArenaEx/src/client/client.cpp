#include <iostream>
#include <vector>
#include <hooks/pacemaker.h>
#include <hooks/loadingdone.h>
#include <hooks/random.h>

#include "client.h"

void client::Send(unsigned char id, std::vector<unsigned char> data) {
	data.insert(data.begin(), id);
	int buffer_length = data.size();

	ENetPacket* packet = enet_packet_create(&data[0], data.size(), ENET_PACKET_FLAG_RELIABLE);
	if (peer != NULL) {
		enet_peer_send(peer, 0, packet);
		enet_host_flush(client);
	}
}

void client::Send(unsigned char id, std::string msg) {
	std::vector<unsigned char> data;
	data.insert(data.begin(), msg.begin(), msg.end());
	Send(id, data);
}

void client::SendWithRandom(unsigned char id, std::string msg) {
	std::vector<unsigned char> data;
	data.resize(7 * sizeof(unsigned int));
	EnterCriticalSection(&hooks::random::RandomCriticalSection);
	std::memcpy(data.data(), &hooks::random::current_random, 7 * sizeof(unsigned int));
	LeaveCriticalSection(&hooks::random::RandomCriticalSection);
	data.insert(data.end(), msg.begin(), msg.end());
	Send(id, data);
}

void ParsePacket(std::vector<unsigned char> data) {
	unsigned char id = data.front();
	data.erase(data.begin());
	switch (id)
	{
	case 1:
		if (data.size() <= 0 || data.size() > sizeof(unsigned int)) {
			break;
		}
		fprintf(stdout, "datasize : %u\n", data.size());
		hooks::pacemaker::p2_score = 0;
		memcpy(&hooks::pacemaker::p2_score, &data[0], data.size()); // little-endian, kinda ugly but it works(tm)
		if (hooks::pacemaker::pacemaker_address)
			*hooks::pacemaker::pacemaker_address = hooks::pacemaker::p2_score;
		if (hooks::pacemaker::pacemaker_display_address)
			*hooks::pacemaker::pacemaker_display_address = hooks::pacemaker::p2_score;
		fprintf(stdout, "p2score : %u\n", hooks::pacemaker::p2_score);
		break;
	case 2:
		fprintf(stdout, "p2 ready\n");
		hooks::loading_done::is_p2_ready = true;
		break;
	case 3:
		if (data.size() != 7 * 4) {
			fprintf(stderr, "invalid size random\n");
			break;
		}
		fprintf(stdout, "received random\n");
		hooks::random::received_random = true;
		EnterCriticalSection(&hooks::random::RandomCriticalSection);
		memcpy(&hooks::random::current_random, &data[0], data.size());
		LeaveCriticalSection(&hooks::random::RandomCriticalSection);
		break;
	case 4:
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
	default:
		break;
	}
}

DWORD WINAPI client::ListenLoop(LPVOID lpParam) {
	ENetEvent event;
	std::vector<unsigned char> data;

    while (enet_host_service(client, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
            printf("A new client connected from %x:%u.\n",
                event.peer->address.host,
                event.peer->address.port);

            /* Store any relevant client information here. */
            event.peer->data = "Client information";

            break;
        case ENET_EVENT_TYPE_RECEIVE:
            printf("A packet of length %u containing %s was received from %s on channel %u.\n",
                event.packet->dataLength,
                event.packet->data,
                event.peer->data,
                event.channelID);

			data.assign((unsigned char)(event.packet->data), event.packet->dataLength);
			ParsePacket(data);
            /* Clean up the packet now that we're done using it. */
            enet_packet_destroy(event.packet);

            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            printf("%s disconnected.\n", event.peer->data);

            /* Reset the peer's client information. */

            event.peer->data = NULL;
        }
    }
}

bool client::Init() {
	if (enet_initialize() != 0) {
		std::cout << "[!] Error initalizing ENet" << std::endl;
		return false;
	}
	atexit(enet_deinitialize);

	client = enet_host_create(NULL /* create a client host */,
		1 /* only allow 1 outgoing connection */,
		2 /* allow up 2 channels to be used, 0 and 1 */,
		0 /* assume any amount of incoming bandwidth */,
		0 /* assume any amount of outgoing bandwidth */);

	if (client == NULL)
	{
		std::cout << "[!] Error creating ENet client" << std::endl;
		return false;
	}

    DWORD clientReceiverId;
    clientReceiverHandle = CreateThread(NULL, 0, ListenLoop, NULL, 0, &clientReceiverId);
    if (clientReceiverHandle == NULL)
    {
        std::cout << "[!] run::CreateThread clientListenLoop failed" << std::endl;
        return false;
    }
}

bool client::Destroy() {
    TerminateThread(clientReceiverHandle, 0);
	enet_host_destroy(client);
	return true;
}