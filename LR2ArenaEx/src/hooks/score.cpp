#include <iostream>
#include <utils/mem.h>
#include <client/client.h>
#include <network/enums.h>

#include "score.h"

DWORD WINAPI ScoreConsumer(LPVOID lpParameter)
{
	std::cout << "[i] Started ScoreConsumer thread" << std::endl;
	while (true) {
		hooks::score::ScoreEvent score_event;
		hooks::score::score_queue.wait_dequeue(score_event);

		fprintf(stdout, "poor:%d bad:%d good:%d great:%d p_great:%d max_combo:%d score:%d\n",
			score_event.poor, score_event.bad, score_event.good, score_event.great, score_event.p_great,
			score_event.max_combo, score_event.score
		);

		std::vector<unsigned char> score_data;
		score_data.resize(sizeof(score_event));
		std::memcpy(score_data.data(), &score_event, score_data.size());
		client::Send(network::ClientToServer::SEND_PLAYER_SCORE, score_data);
	}
}

void hkScore(unsigned char* memory) {
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

	hooks::score::ScoreEvent score_event{
		poor,
		bad,
		good,
		great,
		p_great,
		max_combo,
		score
	};
	hooks::score::score_queue.enqueue(score_event);
}

DWORD jmp_lr2body_406226 = 0x406226;
__declspec(naked) void trampScore() {

	__asm {
		add[esi + eax * 4 + 620964], edi
		// hook esi + ((index*4) + 0x979A4) = score address
		pushfd
		pushad
		push esi
		call hkScore
		add esp, 4
		popad
		popfd
		// end hook
		add esp, 4
		//call call_lr2body_405FB0 // 00418840 | E8 6BD7FEFF | call lr2body.405FB0 |
		jmp jmp_lr2body_406226
	}
}

void hooks::score::Setup() {
	DWORD score_thread_id;
	HANDLE score_thread_handle = CreateThread(NULL, 0, ScoreConsumer, NULL, 0, &score_thread_id);
	if (score_thread_handle == NULL)
	{
		std::cout << "[!] run::CreateThread score_thread_handle failed" << std::endl;
		return;
	}

	mem::HookFn((char*)0x40621F, (char*)trampScore, 7);
}