#include <iostream>
#include <vector>
#include <hooks/pacemaker.h>
#include <hooks/loadingdone.h>
#include <hooks/random.h>

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

void client::SendWithRandom(network::ClientToServer id, std::string msg) {
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
	std::vector<unsigned char> data;
	data.reserve(MAX_TCP);

    while (true)
    {
		if (!connected)
			break;
		client.receive(&data[0], MAX_TCP);
		std::cout << &data[0] << std::endl;
		ParsePacket(data);
    }
}

bool client::Connect(const char* host) {

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