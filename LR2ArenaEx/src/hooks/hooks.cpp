#include <utils/mem.h>

#include "hooks.h"

#include <windows.h>
#include <iostream>
#include <psapi.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <codecvt>
#include <fstream>
#include <vector>
#include <winsock.h>
#include <queue>

#include <readerwriterqueue/readerwriterqueue.h>

#pragma comment(lib, "ws2_32.lib")

#define ENABLE_DEBUG
#define ENABLE_BROADCAST
const int MAX_UDP = 500;

unsigned int p2_score = 0;
static BOOL is_p2_ready = false;
static BOOL received_random = false;
static BOOL random_flip = false;
static BOOL is_returning_to_menu = false;
static unsigned int* pacemaker_address = NULL;
static unsigned int* pacemaker_display_address = NULL;
static unsigned int current_random[] = { 0, 0, 0, 0, 0, 0, 0 };
std::string selected_bms;
CRITICAL_SECTION RandomCriticalSection;

struct ScoreEvent {
	int poor;
	int bad;
	int good;
	int great;
	int p_great;
	int max_combo;
	int score;
};
moodycamel::BlockingReaderWriterQueue<ScoreEvent> score_queue(1000);


#ifdef ENABLE_BROADCAST

SOCKET broadcast_socket_send;
SOCKET broadcast_socket_recv;
struct sockaddr_in sendto_addr;
struct sockaddr_in recv_addr;
u_short broadcast_port_send = 2222;
u_short broadcast_port_recv = 2223;

void initialize_broadcast(SOCKET* socketOut, u_short port, struct sockaddr_in* addr, BOOL send) {

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	*socketOut = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	if (send) {
		addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);; // inet_addr(broadcast_address);
		return;
	}
	addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if (bind(*socketOut, (sockaddr*)addr, sizeof(sockaddr)) < 0) {
		fprintf(stderr, "error binding socket\n");
		closesocket(*socketOut);
		return;
	}
}

std::vector<unsigned char> recv_broadcast() {
	std::vector<unsigned char> data;
	sockaddr_in addr;
	int buffer_length = MAX_UDP;
	char buffer[MAX_UDP];
	int from_length = sizeof(sockaddr);
	int result = recvfrom(broadcast_socket_recv, buffer, buffer_length, 0, (sockaddr*)&addr, &from_length);
	if (result == SOCKET_ERROR) {
		fprintf(stderr, "result == SOCKET_ERROR\n");
		return data;
	}
	for (int i = 0; i < result; i++) {
		data.push_back(buffer[i]);
	}
	return data;
}

DWORD WINAPI receiver_consumer(LPVOID lpParameter) {
	while (true) {
		std::vector<unsigned char> data = recv_broadcast();
		unsigned char id = data.front();
		data.erase(data.begin());
		switch (id)
		{
		case 1:
			if (data.size() <= 0 || data.size() > sizeof(unsigned int)) {
				continue;
			}
			fprintf(stdout, "datasize : %u\n", data.size());
			p2_score = 0;
			memcpy(&p2_score, &data[0], data.size()); // little-endian, kinda ugly but it works(tm)
			if (pacemaker_address)
				*pacemaker_address = p2_score;
			if (pacemaker_display_address)
				*pacemaker_display_address = p2_score;
			fprintf(stdout, "p2score : %u\n", p2_score);
			break;
		case 2:
			fprintf(stdout, "p2 ready\n");
			is_p2_ready = true;
			break;
		case 3:
			if (data.size() != 7 * 4) {
				fprintf(stderr, "invalid size random\n");
				continue;
			}
			fprintf(stdout, "received random\n");
			received_random = true;
			EnterCriticalSection(&RandomCriticalSection);
			memcpy(&current_random, &data[0], data.size());
			LeaveCriticalSection(&RandomCriticalSection);
			break;
		case 4:
			random_flip = data.front() == 1;
			if (random_flip) {
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
}

void deitialize_broadcast(SOCKET socket) {
	closesocket(socket);
}

void broadcast(unsigned char id, std::vector<unsigned char> data) {
	data.insert(data.begin(), id);
	int buffer_length = data.size();
	if (buffer_length > MAX_UDP) {
#ifdef ENABLE_DEBUG
		fprintf(stderr, "broadcast:: buffer_length > MAX_UDP\n");
#endif
		return;
	}
	char* buffer = new char[buffer_length];
	memcpy(buffer, data.data(), buffer_length);
	if (sendto(broadcast_socket_send, buffer, buffer_length, 0, (sockaddr*)&sendto_addr, sizeof(sendto_addr)) < 0) {
		fprintf(stderr, "sendto\n");
	}
	delete[] buffer;
}

void broadcast(unsigned char id, std::string msg) {
	std::vector<unsigned char> data;
	data.insert(data.begin(), msg.begin(), msg.end());
	broadcast(id, data);
}

void broadcast_with_random(unsigned char id, std::string msg) {
	std::vector<unsigned char> data;
	data.resize(7 * sizeof(unsigned int));
	EnterCriticalSection(&RandomCriticalSection);
	std::memcpy(data.data(), &current_random, 7 * sizeof(unsigned int));
	LeaveCriticalSection(&RandomCriticalSection);
	data.insert(data.end(), msg.begin(), msg.end());
	broadcast(id, data);
}

#endif

int (*get_rng)(int) = (int(*)(int))0x6C95E0; // rng address
void update_random() {
	EnterCriticalSection(&RandomCriticalSection);
	for (int i = 6; i >= 0; i--) {
		current_random[i] = get_rng(i);
		fprintf(stdout, "random %d: %d\n", i, current_random[i]);
	}
	LeaveCriticalSection(&RandomCriticalSection);
}

void select_bms(const char** buffer, unsigned char* memory) {
	unsigned int selected_option = (unsigned int)*(memory + 0x10);
	std::string new_selected_bms = std::string(*buffer);
	if (!new_selected_bms.rfind("LR2files\\Config\\sample_", 0)) {
		fprintf(stdout, "demo BMS loaded, skip\n");
		return;
	}
	selected_bms = new_selected_bms;
	p2_score = 0; // it's most likely when you start a song

#ifdef ENABLE_DEBUG
	fprintf(stdout, "select_bms: %s\n", selected_bms.c_str());
	fprintf(stdout, "selected_option: %d\n", selected_option);
#endif
	if (!received_random) {
		update_random();
	}
#ifdef ENABLE_BROADCAST
	broadcast_with_random(1, selected_bms);
#endif
	is_returning_to_menu = false;
}

void swap_two_lanes(unsigned int* random, int i, int j) {
	unsigned int tmp = random[i];
	random[i] = random[j];
	random[j] = tmp;
}

void mirror_random(unsigned int* random) {
	for (int i = 0; i < 7; i++) {
		random[i] = 8 - random[i];
	}
}

int random(int range, unsigned char* memory) {
	EnterCriticalSection(&RandomCriticalSection);
	int rand = current_random[range];
	LeaveCriticalSection(&RandomCriticalSection);
	if (range == 1 && random_flip) { // last call to hook_random
		unsigned int* random_addr = (unsigned int*)(memory + 476);
		fprintf(stdout, "flipping random at address %p\n", random_addr);

		if (rand == 1) {
			swap_two_lanes(random_addr, 5, 6);
		}
		mirror_random(random_addr);
		if (rand == 1) {
			swap_two_lanes(random_addr, 5, 6); // neutralize swap made by LR2 later
		}
	}
	return rand;
}

__declspec(naked) void hook_random() {

	__asm {
		// hook ecx = ptr ptr bms path
		pushfd
		push ecx
		push edx
		push ebx
		push esp
		push ebp
		push esi
		push edi
		// args
		push esp // random is at [esp+476]
		push edi
		call random
		add esp, 8
		//
		pop edi
		pop esi
		pop ebp
		pop esp
		pop ebx
		pop edx
		pop ecx
		popfd
		ret
		// end hook
	}
}

DWORD call_lr2body_715DA0 = 0x715DA0;
__declspec(naked) void hook_select_bms() {

	__asm {
		// hook ecx = ptr ptr bms path
		pushfd
		pushad
		push[esp + 0x4c] // selected option is at [[esp+0x4c] + 0x10]
		push ecx
		call select_bms
		add esp, 8
		popad
		popfd
		// end hook
		mov edi, edi
		push ebp
		mov ebp, esp
		push 0x40
		push dword ptr ss : [ebp + 0xC]
		push dword ptr ss : [ebp + 0x8]
		call call_lr2body_715DA0 // 00715E71 | E8 2AFFFFFF | call lr2body.715DA0 |
		add esp, 0xC
		pop ebp
		ret
	}
}

DWORD WINAPI score_consumer(LPVOID lpParameter)
{
#ifdef _DEBUG
	fprintf(stdout, "Trace: score_consumer - started\n");
#endif
	while (true) {
		ScoreEvent score_event;
		score_queue.wait_dequeue(score_event);
#ifdef ENABLE_DEBUG
		fprintf(stdout, "poor:%d bad:%d good:%d great:%d p_great:%d max_combo:%d score:%d\n",
			score_event.poor, score_event.bad, score_event.good, score_event.great, score_event.p_great,
			score_event.max_combo, score_event.score
		);
#endif
#ifdef ENABLE_BROADCAST
		std::vector<unsigned char> score_data;
		score_data.resize(sizeof(score_event));
		std::memcpy(score_data.data(), &score_event, score_data.size());
		broadcast(2, score_data);
#endif
	}
}

void score(unsigned char* memory) {
	memory += 0x979A4;
	int* ptr = (int*)memory;

	int max_combo = ptr[-3];
	int poor = ptr[1];
	int bad = ptr[2];
	int good = ptr[3];
	int great = ptr[4];
	int p_great = ptr[5];
	int score = ptr[45];
	//int score1 = ptr[46];
	//fprintf(stdout, "max_combo:%d\n", max_combo);

	ScoreEvent score_event{
		poor,
		bad,
		good,
		great,
		p_great,
		max_combo,
		score
	};
	score_queue.enqueue(score_event);
}

void pacemaker(unsigned char* memory) {
	memory += 0x4C;
	if (!pacemaker_address) {
		*(unsigned int*)memory = p2_score;
		pacemaker_address = (unsigned int*)memory;
	}
}

void pacemaker_display(unsigned char* memory) {
	memory += 0xD8;
	if (!pacemaker_display_address) {
		*(unsigned int*)memory = p2_score;
		pacemaker_display_address = (unsigned int*)memory;
	}
}

void send_player_ready() {
	if (!is_returning_to_menu) {
		broadcast(4, ""); // player ready
		fprintf(stdout, "waiting for p2\n");
		while (!is_p2_ready)
		{
			SleepEx(50, false);
		}
		is_p2_ready = false;
		received_random = false;
		fprintf(stdout, "lock released\n");
	}
	else {
		is_returning_to_menu = false; // might not be needed because of reset in select_bms but keeping it just in case
	}
}

void send_escaped() {
	fprintf(stdout, "returning to menu\n");
	is_returning_to_menu = true;
	broadcast(3, ""); // escaped
}

DWORD jmp_lr2body_406226 = 0x406226;
__declspec(naked) void hook_score() {

	__asm {
		add[esi + eax * 4 + 620964], edi
		// hook esi + ((index*4) + 0x979A4) = score address
		pushfd
		pushad
		push esi
		call score
		add esp, 4
		popad
		popfd
		// end hook
		add esp, 4
		//call call_lr2body_405FB0 // 00418840 | E8 6BD7FEFF | call lr2body.405FB0 |
		jmp jmp_lr2body_406226
	}
}

DWORD jmp_lr2body_4A880A = 0x4A880A;
__declspec(naked) unsigned int hook_pacemaker() {

	__asm {
		// hook [ecx+4Ch] = pacemaker score address
		pushfd
		pushad
		push ecx
		call pacemaker
		add esp, 4
		popad
		popfd
		// end hook
		mov eax, 1
		jmp jmp_lr2body_4A880A
	}
}

DWORD jmp_lr2body_4A9F07 = 0x4A9F07;
__declspec(naked) unsigned int hook_pacemaker_display() {

	__asm {
		// hook [esi+0D8h] = pacemaker display score address
		pushfd
		pushad
		push esi
		call pacemaker_display
		add esp, 4
		popad
		popfd
		// end hook
		add esp, 4
		jmp jmp_lr2body_4A9F07
	}
}

DWORD jmp_lr2body_40848D = 0x40848D;
__declspec(naked) unsigned int hook_loading_done() {

	__asm {
		// hook [esi+97BAAh] = loading done
		pushfd
		pushad
		call send_player_ready
		popad
		popfd
		// end hook
		mov byte ptr[esi + 97BAAh], 1
		add esp, 4
		jmp jmp_lr2body_40848D
	}
}

DWORD jmp_lr2body_419863 = 0x419863;
__declspec(naked) unsigned int hook_return_menu() {

	__asm {
		// hook [esi+97BA9h] = return menu
		pushfd
		pushad
		call send_escaped
		popad
		popfd
		// end hook
		mov byte ptr[esi + 97BA9h], 1
		add esp, 4
		jmp jmp_lr2body_419863
	}
}

bool hooks::SetupHooks() {
#ifdef ENABLE_BROADCAST
	DWORD pacemaker_thread_id;

	initialize_broadcast(&broadcast_socket_send, broadcast_port_send, &sendto_addr, true);
	initialize_broadcast(&broadcast_socket_recv, broadcast_port_recv, &recv_addr, false);
	HANDLE pacemaker_thread_handle = CreateThread(NULL, 0, receiver_consumer, NULL, 0, &pacemaker_thread_id);
	if (pacemaker_thread_handle == NULL)
	{
		std::cout << "[!] run::CreateThread pacemaker_thread_handle failed" << std::endl;
		return false;
	}
#endif

	// init
	selected_bms = std::string();
	InitializeCriticalSection(&RandomCriticalSection);

	DWORD score_thread_id;
	HANDLE score_thread_handle = CreateThread(NULL, 0, score_consumer, NULL, 0, &score_thread_id);
	if (score_thread_handle == NULL)
	{
		std::cout << "[!] run::CreateThread score_thread_handle failed" << std::endl;
		return false;
	}

	mem::HookFn((char*)0x4B0D92, (char*)hook_select_bms, 5);
	mem::HookFn((char*)0x40621F, (char*)hook_score, 7);
	mem::HookFn((char*)0x4A8802, (char*)hook_pacemaker, 8);
	mem::HookFn((char*)0x4A9F01, (char*)hook_pacemaker_display, 6);
	mem::WriteMemory((LPVOID)0x4A9F7B, (char*)"\x90\x90\x90\x90\x90\x90", 6); // nop pacemaker display update at the end
	mem::HookFn((char*)0x4B4698, (char*)hook_random, 5);
	mem::HookFn((char*)0x408486, (char*)hook_loading_done, 7);
	mem::HookFn((char*)0x41985C, (char*)hook_return_menu, 7);

	return true;
}

void hooks::Destroy() {
#ifdef ENABLE_BROADCAST
	deitialize_broadcast(broadcast_socket_send);
	deitialize_broadcast(broadcast_socket_recv);
	WSACleanup();
#endif
	DeleteCriticalSection(&RandomCriticalSection);
}