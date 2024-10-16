#include <msgpack/msgpack.hpp>
#include <iostream>
#include <utils/mem.h>
#include <client/client.h>
#include <network/enums.h>
#include <network/structs.h>
#include <filesystem>
#include <sqlite_modern_cpp.h>
#include <utils/misc.h>

#include "selectbms.h"
#include "pacemaker.h"
#include "random.h"
#include "returnmenu.h"

void SendWithRandom(network::SelectedBmsMessage msg) {
	EnterCriticalSection(&hooks::random::RandomCriticalSection);
	msg.random = hooks::random::current_random;
	LeaveCriticalSection(&hooks::random::RandomCriticalSection);

	auto msgPack = msgpack::pack(msg);

	client::Send(network::ClientToServer::CTS_SELECTED_BMS, msgPack);
}

std::string GetDatabasePath()
{
	WCHAR dllPath[MAX_PATH];
	GetModuleFileNameW(NULL, dllPath, MAX_PATH);
	std::filesystem::path wPath(dllPath);
	wPath.remove_filename();
	wPath = wPath / "LR2files" / "Database" / "song.db";
	return wPath.u8string();
}

network::SelectedBmsMessage GetBmsInfo(std::string bmsPath) {
	std::string dbPath = GetDatabasePath();

	sqlite::sqlite_config config;
	config.flags = sqlite::OpenFlags::READONLY;
	try {
		sqlite::database db(dbPath);
		std::string hash, title, subtitle, artist, subartist;
		db << "select hash, title, subtitle, artist, subartist from song where path = ?" << bmsPath >> std::tie(hash, title, subtitle, artist, subartist);
		std::cout << "[+] Hash: " << hash << std::endl;
		std::cout << "[+] Title: " << title << " " << subtitle << std::endl;
		std::cout << "[+] Artist: " << artist << " " << subartist << std::endl;

		std::string bmsInfo;
		std::string delimiter = "\xff"; // use as a string delimiter (guaranteed to not be used in UTF-8)
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
	unsigned int selected_option = (unsigned int)*(memory + 0x10);
	std::string selectedBms = std::string(*buffer);
	if (!selectedBms.rfind("LR2files\\Config\\sample_", 0)) {
		fprintf(stdout, "demo BMS loaded, skip\n");
		return;
	}
	hooks::pacemaker::displayed_score = 0; // it's most likely when you start a song so reset score

	std::string selectedBmsUtf8 = utils::SJISToUTF8(selectedBms);

	std::cout << "[+] Selected BMS: " << selectedBmsUtf8 << std::endl;
	std::cout << "[+] Selected option: " << selected_option << std::endl;

	if (!hooks::random::received_random) {
		hooks::random::UpdateRandom();
	}

	auto bmsInfo = GetBmsInfo(selectedBmsUtf8);
	if (bmsInfo.hash.length() > 0)
		SendWithRandom(bmsInfo);

	hooks::return_menu::is_returning_to_menu = false;
}

DWORD fsopen_addr = 0x715DA0;
__declspec(naked) void trampSelectBms() {
	__asm {
		// hook ecx = ptr ptr bms path
		pushfd
		pushad
		push[esp + 0x4c] // selected option is at [[esp+0x4c] + 0x10]
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