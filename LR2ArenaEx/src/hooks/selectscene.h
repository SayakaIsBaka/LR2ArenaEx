#pragma once

namespace hooks {
	namespace select_scene {
		typedef int(__cdecl* ProcSelect)(void*, void*);
		inline ProcSelect oProcSelect;
		inline void* game = NULL;

		void Setup();
		void SearchSongByHash(std::string hash);
	}
}