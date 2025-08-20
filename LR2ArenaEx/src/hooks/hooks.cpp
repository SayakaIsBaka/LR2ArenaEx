#include <utils/mem.h>
#include <client/client.h>

#include "hooks.h"
#include "selectbms.h"
#include "score.h"
#include "pacemaker.h"
#include "random.h"
#include "loadingdone.h"
#include "returnmenu.h"
#include "maxscore.h"
#include "currentnotes.h"
#include "midi.h"
#include "maniac.h"
#include "fmod.h"
#include "selectscene.h"

bool hooks::SetupHooks() { // Pacemaker hook is missing from here as we only hook when connecting
	hooks::select_bms::Setup();
	hooks::score::Setup();
	hooks::random::Setup();
	hooks::loading_done::Setup();
	hooks::return_menu::Setup();
	hooks::max_score::Setup();
	hooks::current_notes::Setup();
	hooks::midi::Setup();
	hooks::maniac::Setup();
	hooks::fmod::Setup();
	hooks::select_scene::Setup();

	return true;
}

void hooks::Destroy() {
	client::Disconnect();
	client::Destroy();
	hooks::random::Destroy();
}