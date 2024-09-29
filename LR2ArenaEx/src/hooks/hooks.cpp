#include <utils/mem.h>
#include <client/client.h>

#include "hooks.h"
#include "selectbms.h"
#include "score.h"
#include "pacemaker.h"
#include "random.h"
#include "loadingdone.h"
#include "returnmenu.h"

bool hooks::SetupHooks() {
	client::Init();

	hooks::select_bms::Setup();
	hooks::score::Setup();
	hooks::pacemaker::Setup();
	hooks::random::Setup();
	hooks::loading_done::Setup();
	hooks::return_menu::Setup();

	return true;
}

void hooks::Destroy() {
	client::Destroy();
	hooks::random::Destroy();
}