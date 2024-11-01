#pragma once

#include <network/structs.h>
#include <utils/keys.h>
#include <dinput.h>
#include <vector>
#include <fonts/IconsFontAwesome6.h>
#include <random>
#include <unordered_map>

namespace hooks {
	namespace maniac {
		struct Item {
			std::string name;
			std::string icon;
			unsigned int* address;
			unsigned int defaultVal;
			unsigned int lv1;
			unsigned int lv2;
			unsigned int lv3;
		};

		inline std::vector<Item> items {
			{ "Earthquake", ICON_FA_HOUSE_CRACK, (unsigned int*)0xff8e0, 0, 5, 10, 15 },
			{ "Tornado", ICON_FA_TORNADO, (unsigned int*)0xff8e4, 0, 5, 10, 15 },
			{ "Superloop", ICON_FA_ROTATE, (unsigned int*)0xff8e8, 0, 5, 10, 15 },
			{ "Char", ICON_FA_ANGLES_DOWN, (unsigned int*)0xff8f0, 0, 5, 10, 15 },
			{ "Heartbeat", ICON_FA_HEART_PULSE, (unsigned int*)0xff8f4, 0, 5, 10, 15 },
			{ "Nabeatsu", ICON_FA_FACE_DIZZY, (unsigned int*)0xff904, 0, 5, 10, 15 },
			{ "Acceleration", ICON_FA_PERSON_RUNNING, (unsigned int*)0xff908, 0, 1, 2, 3 },
			{ "Sin curve", ICON_FA_WAVE_SQUARE, (unsigned int*)0xff90c, 0, 5, 10, 15 },
			{ "Wave", ICON_FA_WATER, (unsigned int*)0xff910, 0, 5, 10, 15 },
			{ "Spiral", ICON_FA_ARROWS_SPIN, (unsigned int*)0xff914, 0, 5, 10, 15 },
			{ "Sidejump", ICON_FA_ARROWS_TURN_TO_DOTS, (unsigned int*)0xff918, 0, 5, 10, 15 },
		};

		constexpr inline int itemTime = 8000; // in ms

		inline bool itemModeEnabled = false;
		inline unsigned int currentCombo = 0;
		inline unsigned int threshold = 100; // Default item threshold
		inline network::CurrentItem currentItem;
		inline utils::keys::Key itemKeyBind(utils::keys::DeviceType::KEYBOARD, DIK_BACKSPACE); // Default binding

		inline std::random_device dev;
		inline std::mt19937 rng;
		inline std::uniform_int_distribution<std::mt19937::result_type> dist;

		inline std::unordered_map<network::CurrentItem, int> activeItems; // contains remaining time in ms

		void SaveToConfigFile();
		void LoadConfig(std::string type, std::string value);
		void ResetState();
		void UseItem();
		void Setup();
		void TriggerItem(network::CurrentItem item);
	}
}