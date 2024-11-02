#include <msgpack/msgpack.hpp>
#include <config/config.h>
#include <utils/mem.h>
#include <iostream>
#include <thread>
#include <client/client.h>

#include "maniac.h"
#include "maxscore.h"

void hooks::maniac::SaveToConfigFile() {
	config::SetConfigValue("controller_type", std::to_string(static_cast<unsigned int>(itemKeyBind.type)));
	config::SetConfigValue("item_keybind", std::to_string(itemKeyBind.value));
	config::SaveConfig();
}

void hooks::maniac::LoadConfig(std::string type, std::string value) {
	try {
		int intType = std::stoi(type);
		int intVal = std::stoi(value);
		utils::keys::DeviceType t;

		if (intVal < 0)
			throw std::invalid_argument("Invalid value");

		switch (intType) {
		case 2:
			t = utils::keys::DeviceType::KEYBOARD;
			if (intVal >= 0xEE)
				throw std::invalid_argument("Invalid value");
			break;
		case 3:
			t = utils::keys::DeviceType::CONTROLLER;
			if (intVal >= 32)
				throw std::invalid_argument("Invalid value");
			break;
		case 4:
			t = utils::keys::DeviceType::MIDI;
			if (intVal >= 128)
				throw std::invalid_argument("Invalid value");
			break;
		default:
			throw std::invalid_argument("Invalid device type");
		}

		itemKeyBind = utils::keys::Key(t, intVal);
	}
	catch (std::exception e) {
		std::cout << "[!] Error parsing config file for key bindings, falling back to default" << std::endl;
	}
}

void ApplyItemEffect(network::CurrentItem item) {
	auto i = hooks::maniac::items[item.rolledItemId];
	switch (item.level) {
	case 1:
		*(i.address) = i.lv1;
		break;
	case 2:
		*(i.address) = i.lv2;
		break;
	case 3:
		*(i.address) = i.lv3;
		break;
	default:
		break;
	}
}

void ResetItemEffect(network::CurrentItem item) {
	auto i = hooks::maniac::items[item.rolledItemId];
	auto numMatchItemsType = std::count_if(hooks::maniac::activeItems.begin(), hooks::maniac::activeItems.end(),
		[&item](std::pair<network::CurrentItem, int> const& x) { return x.first.rolledItemId == item.rolledItemId; });
	if (numMatchItemsType == 1) // If only one item type restore otherwise do nothing
		*(i.address) = i.defaultVal;
}

void TriggerItemThread(network::CurrentItem item) {
	using namespace std::chrono_literals;

	hooks::maniac::activeItems.insert({ item, hooks::maniac::itemTime });
	ApplyItemEffect(item);

	while (hooks::maniac::activeItems[item] > 0) {
		hooks::maniac::activeItems[item] -= 10;
		std::this_thread::sleep_for(10ms);
	}

	ResetItemEffect(item);
	hooks::maniac::activeItems.erase(item);
}

void hooks::maniac::TriggerItem(network::CurrentItem item) {
	if (item.rolledItemId < 0 || item.rolledItemId >= items.size() || item.level < 1 || item.level > 3) {
		std::cout << "[!] Invalid item received" << std::endl;
		return;
	}

	if (activeItems.count(item)) { // If item already active add time
		activeItems[item] += itemTime;
	}
	else {
		std::thread t(TriggerItemThread, item);
		t.detach();
	}
}

void SendItem(network::CurrentItem item) {
	auto buf = msgpack::pack(item);
	client::Send(network::ClientToServer::CTS_ITEM, buf);
}

void hooks::maniac::UseItem() {
	if (currentItem.rolledItemId < 0) // If no item
		return;
	std::thread t(SendItem, hooks::maniac::currentItem);
	t.detach();
	ResetState();
}

void hooks::maniac::ResetState() {
	currentCombo = 0;
	currentItem = network::CurrentItem();
}

void hkResetCombo() {
	hooks::maniac::currentCombo = 0;
}

void hkUpdateCombo() {
	if (hooks::max_score::maxScore != 0)
		hooks::maniac::threshold = std::round((hooks::max_score::maxScore / 2) * hooks::maniac::thresholdMult); // Determine threshold from the number of total notes in chart

	hooks::maniac::currentCombo++;
	if (hooks::maniac::currentCombo == hooks::maniac::threshold) {
		hooks::maniac::currentItem.rolledItemId = hooks::maniac::dist(hooks::maniac::rng);
		hooks::maniac::currentItem.level = 1;
	}
	else if (hooks::maniac::currentCombo == hooks::maniac::threshold * 2) {
		hooks::maniac::currentItem.level = 2;
	}
	else if (hooks::maniac::currentCombo == hooks::maniac::threshold * 3) {
		hooks::maniac::currentItem.level = 3;
	}
}

DWORD jmp_lr2body_406440 = 0x406440;
__declspec(naked) unsigned int trampResetCombo() {

	__asm {
		pushfd
		pushad
		call hkResetCombo
		popad
		popfd
		// end hook
		mov [edi + 97990h], edx
		add esp, 4
		jmp jmp_lr2body_406440
	}
}

DWORD jmp_lr2body_40640A = 0x40640A;
__declspec(naked) unsigned int trampUpdateCombo() {

	__asm {
		pushfd
		pushad
		call hkUpdateCombo
		popad
		popfd
		// end hook
		add [edi + 97990h], ecx
		add esp, 4
		jmp jmp_lr2body_40640A
	}
}

void hooks::maniac::UpdateItemWeights() {
	std::vector<unsigned int> weights;
	std::for_each(items.begin(), items.end(), [&weights](Item x) { weights.push_back(x.weight); });
	dist = std::discrete_distribution<std::mt19937::result_type>(weights.begin(), weights.end());
}

void hooks::maniac::Setup() {
	// Init RNG
	rng = std::mt19937(dev());
	UpdateItemWeights();

	mem::HookFn((char*)0x406404, (char*)trampUpdateCombo, 6);
	mem::HookFn((char*)0x40643A, (char*)trampResetCombo, 6);
}