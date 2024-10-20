#include <filesystem>
#include <sqlite_modern_cpp.h>
#include <network/enums.h>

#include "misc.h"

unsigned int utils::CalculateExScore(network::Score score) {
	return score.p_great * 2 + score.great;
}

// From imgui_demo.cpp
void utils::StrTrim(char* s) {
	char* str_end = s + strlen(s);
	while (str_end > s && isspace(str_end[-1]))
		str_end--;
	*str_end = 0;
}

// https://gist.github.com/takamin/2752a45c5cb4d0f9d1ff
std::string utils::SJISToUTF8(const std::string& sjis) {
	std::string utf8_string;

	LPCCH pSJIS = (LPCCH)sjis.c_str();
	int utf16size = ::MultiByteToWideChar(932, MB_ERR_INVALID_CHARS, pSJIS, -1, 0, 0);
	if (utf16size != 0) {
		LPWSTR pUTF16 = new WCHAR[utf16size];
		if (::MultiByteToWideChar(932, 0, (LPCCH)pSJIS, -1, pUTF16, utf16size) != 0) {
			int utf8size = ::WideCharToMultiByte(CP_UTF8, 0, pUTF16, -1, 0, 0, 0, 0);
			if (utf8size != 0) {
				LPTSTR pUTF8 = new TCHAR[utf8size + 16];
				ZeroMemory(pUTF8, utf8size + 16);
				if (::WideCharToMultiByte(CP_UTF8, 0, pUTF16, -1, pUTF8, utf8size, 0, 0) != 0) {
					utf8_string = std::string(pUTF8);
				}
				delete pUTF8;
			}
		}
		delete pUTF16;
	}
	return utf8_string;
}

std::string utils::GetDatabasePath() {
	WCHAR dllPath[MAX_PATH];
	GetModuleFileNameW(NULL, dllPath, MAX_PATH);
	std::filesystem::path wPath(dllPath);
	wPath.remove_filename();
	wPath = wPath / "LR2files" / "Database" / "song.db";
	return wPath.u8string();
}

std::string utils::GetChartPath(std::string hash) {
	auto dbPath = GetDatabasePath();

	sqlite::sqlite_config config;
	config.flags = sqlite::OpenFlags::READONLY;
	try {
		sqlite::database db(dbPath);
		std::string path;
		db << "select path from song where hash = ? limit 1" << hash >> path;
		return path;
	}
	catch (const sqlite::errors::no_rows& e) { // Chart not found
		return "";
	}
	catch (const std::exception& e) {
		std::cout << "[!] Error: " << e.what() << std::endl;
		return "";
	}
}

std::string utils::GetOptionName(unsigned int option) {
	auto opt = static_cast<network::SelectedOption>(option);
	switch (opt) {
	case network::SelectedOption::MIRROR:
		return "MIRROR";
	case network::SelectedOption::RANDOM:
		return "RANDOM";
	case network::SelectedOption::SRAN:
		return "S-RAN";
	case network::SelectedOption::HRAN:
		return "H-RAN";
	case network::SelectedOption::ALLSCR:
		return "ALLSCR";
	case network::SelectedOption::NONRAN:
	default:
		return "";
	}
}

float utils::CalculateRate(network::Score score, unsigned int maxScore) {
	auto percentage (((float)CalculateExScore(score) / (float)maxScore) * 100.0f);
	return floorf(percentage * 100) / 100; // Round down
}