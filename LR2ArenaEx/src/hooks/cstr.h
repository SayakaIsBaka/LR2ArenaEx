#pragma once

namespace hooks {
	namespace cstr {
		// All of these are actually thiscall so the second param (RDX) isn't actually used
		typedef void(__fastcall* tCstrInit)(char**);
		tCstrInit oCstrInit = (tCstrInit)0x43B630;
		typedef char**(__fastcall* tCstrAssign)(char**, void*, const char*);
		tCstrAssign oCstrAssign = (tCstrAssign)0x43BC10;
		typedef char**(__fastcall* tCstrCopy)(char**, void*, char**);
		tCstrCopy oCstrCopy = (tCstrCopy)0x43BBF0;
		typedef void(__fastcall* tCstrFree)(char**);
		tCstrInit oCstrFree = (tCstrFree)0x43AD80;

		void CstrInit(char** cstr) {
			oCstrInit(cstr);
		}

		char** CstrAssign(char** cstr, const char* str) {
			return oCstrAssign(cstr, 0, str);
		}

		char** CstrCopy(char** dest, char** source) {
			return oCstrCopy(dest, 0, source);
		}

		void CstrFree(char** cstr) {
			oCstrFree(cstr);
		}
	}
}