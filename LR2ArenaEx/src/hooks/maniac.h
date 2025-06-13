#pragma once

#include <network/structs.h>
#include <utils/keys.h>
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
			unsigned int weight;
		};

		inline std::vector<Item> items {
			{ "Earthquake", ICON_FA_HOUSE_CRACK, (unsigned int*)0xff8e0, 0, 15, 30, 50, 50 }, // Easy-mid
			{ "Tornado", ICON_FA_TORNADO, (unsigned int*)0xff8e4, 0, 15, 30, 50, 30 }, // Mid-hard
			{ "Superloop", ICON_FA_ROTATE, (unsigned int*)0xff8e8, 0, 5, 15, 30, 10 }, // Very hard
			{ "Char", ICON_FA_ANGLES_DOWN, (unsigned int*)0xff8f0, 0, 40, 60, 80, 30 }, // Mid
			{ "Heartbeat", ICON_FA_HEART_PULSE, (unsigned int*)0xff8f4, 0, 10, 25, 40, 50 }, // Easy
			{ "Nabeatsu", ICON_FA_FACE_DIZZY, (unsigned int*)0xff904, 0, 25, 50, 100, 30 }, // Mid
			{ "Acceleration", ICON_FA_PERSON_RUNNING, (unsigned int*)0xff908, 0, 1, 2, 3, 30 }, // Mid
			{ "Sin curve", ICON_FA_WAVE_SQUARE, (unsigned int*)0xff90c, 0, 15, 30, 50, 30 }, // Mid
			{ "Wave", ICON_FA_WATER, (unsigned int*)0xff910, 0, 50, 75, 100, 50 }, // Easy
			{ "Spiral", ICON_FA_ARROWS_SPIN, (unsigned int*)0xff914, 0, 20, 35, 50, 30 }, // Mid-hard
			{ "Sidejump", ICON_FA_ARROWS_TURN_TO_DOTS, (unsigned int*)0xff918, 0, 30, 50, 70, 30 }, // Mid
		};

		constexpr inline int itemTime = 8000; // in ms

		inline bool itemModeEnabled = false;
		inline unsigned int currentCombo = 0;
		inline unsigned int threshold = 100; // Default item threshold (should never be used)
		inline float thresholdMult = 0.10f; // Item threshold (given as a percentage of total notes)
		inline bool settingsRemoteUpdated = false; // Hack to update temp variables on GUI
		inline network::CurrentItem currentItem;

		inline std::random_device dev;
		inline std::mt19937 rng;
		inline std::discrete_distribution<std::mt19937::result_type> dist;

		inline std::unordered_map<network::CurrentItem, int> activeItems; // contains remaining time in ms

		void SaveToConfigFile();
		void LoadConfig(std::string type, std::string value);
		void ResetState();
		void UseItem();
		void Setup();
		void UpdateItemWeights();
		void SendItemSettings();
		void UpdateItemSettings(network::ItemSettings settings);
		void TriggerItem(network::CurrentItem item);
	}
}