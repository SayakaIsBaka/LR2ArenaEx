#include <utils/msgpack_utils.h>
#include <iostream>
#include <utils/mem.h>
#include <client/client.h>
#include <network/enums.h>
#include <sqlite_modern_cpp.h>
#include <utils/misc.h>
#include <gui/graph.h>
#include <gui/mainwindow.h>

#include "selectbms.h"
#include "pacemaker.h"
#include "random.h"
#include "returnmenu.h"
#include "maniac.h"

void SendWithRandom(network::SelectedBmsMessage msg) {
	msg.randomSeed = hooks::random::current_seed;
	auto msgPack = msgpack_utils::pack(msg);

	client::Send(network::ClientToServer::CTS_SELECTED_BMS, msgPack);
}

network::SelectedBmsMessage GetBmsInfo(std::string bmsPath) {
	std::string dbPath = utils::GetDatabasePath();

	sqlite::sqlite_config config;
	config.flags = sqlite::OpenFlags::READONLY;
	try {
		sqlite::database db(dbPath);
		std::string hash, title, subtitle, artist, subartist;
		db << "select hash, title, subtitle, artist, subartist from song where path = ?" << bmsPath >> std::tie(hash, title, subtitle, artist, subartist);
		std::cout << "[+] Hash: " << hash << std::endl;
		std::cout << "[+] Title: " << title << " " << subtitle << std::endl;
		std::cout << "[+] Artist: " << artist << " " << subartist << std::endl;

		network::SelectedBmsMessage msg;
		msg.hash = hash;
		msg.title = title + " " + subtitle;
		msg.artist = artist + " " + subartist;
		return msg;
	}
	catch (const std::exception& e) {
		std::cout << "[!] Error: " << e.what() << std::endl;
		return network::SelectedBmsMessage();
	}
}

void hkSelectBms(const char** buffer, unsigned char* memory) {
	if (client::connected) {
		unsigned int selected_option = (unsigned int)*(memory + 0x10);
		unsigned int selected_gauge = *(uintptr_t*)hooks::select_bms::selectedGaugeAddr;
		std::string selectedBms = std::string(*buffer);
		if (!selectedBms.rfind("LR2files\\Config\\sample_", 0)) {
			fprintf(stdout, "demo BMS loaded, skip\n");
			return;
		}
		hooks::pacemaker::displayed_score = 0; // it's most likely when you start a song so reset score

		std::string selectedBmsUtf8 = utils::SJISToUTF8(selectedBms);

		std::cout << "[+] Selected BMS: " << selectedBmsUtf8 << std::endl;
		std::cout << "[+] Selected option: " << selected_option << std::endl;
		std::cout << "[+] Selected gauge: " << selected_gauge << std::endl;

		auto bmsInfo = GetBmsInfo(selectedBmsUtf8);
		bmsInfo.option = selected_option;
		bmsInfo.gauge = selected_gauge;
		bmsInfo.itemModeEnabled = hooks::maniac::itemModeEnabled;

		if (bmsInfo.hash.length() > 0)
			SendWithRandom(bmsInfo);

		if (!(client::state.host == client::state.remoteId) && bmsInfo.hash != client::state.selectedSongRemote.hash)
			gui::main_window::AddToLog("[!] You are not the host; please go back to the main menu and select the same song as the host!");

		if (gui::graph::automaticGraph) {
			gui::graph::showGraph = true; // Show graph on song select
		}

		hooks::return_menu::is_returning_to_menu = false;
		hooks::maniac::ResetState();
	}
}

DWORD fsopen_addr = 0x715DA0;
__declspec(naked) void trampSelectBms() {
	__asm {
		// hook ecx = ptr ptr bms path
		pushfd
		pushad
		push [esp + 0x4c] // selected option is at [[esp+0x4c] + 0x10]
		push ecx
		call hkSelectBms
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
		call fsopen_addr // 00715E71 | E8 2AFFFFFF | call lr2body.715DA0 |
		add esp, 0xC
		pop ebp
		ret
	}
}

void hooks::select_bms::Setup() {
	mem::HookFn((char*)0x4B0D92, (char*)trampSelectBms, 5);
}