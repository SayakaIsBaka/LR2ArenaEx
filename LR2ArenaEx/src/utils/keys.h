#pragma once

#include <unordered_map>
#include <dinput.h>
#include <config/config.h>

namespace utils {
	namespace keys {
		enum class DeviceType : unsigned int {
			NONE = 1,
			KEYBOARD,
			CONTROLLER,
			MIDI,
		};

		struct Key {
			DeviceType type;
			unsigned int value;

			Key(DeviceType type, unsigned int value) {
				this->type = type;
				this->value = value;
			}

			bool operator==(const Key& other) const {
				return (type == other.type && value == other.value);
			};

			Key() {};
		};

		enum class BindingType : unsigned int {
			NONE = 0,
			ITEM_TRIGGER,
			MENU_TOGGLE,
			GRAPH_TOGGLE
		};

		struct Binding {
			std::string id;
			std::string name;
			Key key;
		};

		// Default bindings
		inline std::unordered_map<BindingType, Binding> bindings = {
			{BindingType::ITEM_TRIGGER, {
				"itemTrigger",
				"Item trigger",
				{ utils::keys::DeviceType::KEYBOARD, DIK_BACKSPACE }
			}},
			{BindingType::MENU_TOGGLE, {
				"menuToggle",
				"Menu toggle",
				{ utils::keys::DeviceType::KEYBOARD, DIK_INSERT }
			}},
			{BindingType::GRAPH_TOGGLE, {
				"graphToggle",
				"Graph toggle",
				{ utils::keys::DeviceType::KEYBOARD, DIK_PRIOR }
			}}
		};

		inline const std::unordered_map<unsigned long, std::string> keyToString = {
			{0x01,"ESCAPE"},
			{0x02,"1"},
			{0x03,"2"},
			{0x04,"3"},
			{0x05,"4"},
			{0x06,"5"},
			{0x07,"6"},
			{0x08,"7"},
			{0x09,"8"},
			{0x0A,"9"},
			{0x0B,"0"},
			{0x0C,"MINUS"},
			{0x0D,"EQUALS"},
			{0x0E,"BACK"},
			{0x0F,"TAB"},
			{0x10,"Q"},
			{0x11,"W"},
			{0x12,"E"},
			{0x13,"R"},
			{0x14,"T"},
			{0x15,"Y"},
			{0x16,"U"},
			{0x17,"I"},
			{0x18,"O"},
			{0x19,"P"},
			{0x1A,"LBRACKET"},
			{0x1B,"RBRACKET"},
			{0x1C,"RETURN"},
			{0x1D,"LCONTROL"},
			{0x1E,"A"},
			{0x1F,"S"},
			{0x20,"D"},
			{0x21,"F"},
			{0x22,"G"},
			{0x23,"H"},
			{0x24,"J"},
			{0x25,"K"},
			{0x26,"L"},
			{0x27,"SEMICOLON"},
			{0x28,"APOSTROPHE"},
			{0x29,"GRAVE"},
			{0x2A,"LSHIFT"},
			{0x2B,"BACKSLASH"},
			{0x2C,"Z"},
			{0x2D,"X"},
			{0x2E,"C"},
			{0x2F,"V"},
			{0x30,"B"},
			{0x31,"N"},
			{0x32,"M"},
			{0x33,"COMMA"},
			{0x34,"PERIOD"},
			{0x35,"SLASH"},
			{0x36,"RSHIFT"},
			{0x37,"MULTIPLY"},
			{0x38,"LMENU"},
			{0x39,"SPACE"},
			{0x3A,"CAPITAL"},
			{0x3B,"F1"},
			{0x3C,"F2"},
			{0x3D,"F3"},
			{0x3E,"F4"},
			{0x3F,"F5"},
			{0x40,"F6"},
			{0x41,"F7"},
			{0x42,"F8"},
			{0x43,"F9"},
			{0x44,"F10"},
			{0x45,"NUMLOCK"},
			{0x46,"SCROLL"},
			{0x47,"NUMPAD7"},
			{0x48,"NUMPAD8"},
			{0x49,"NUMPAD9"},
			{0x4A,"SUBTRACT"},
			{0x4B,"NUMPAD4"},
			{0x4C,"NUMPAD5"},
			{0x4D,"NUMPAD6"},
			{0x4E,"ADD"},
			{0x4F,"NUMPAD1"},
			{0x50,"NUMPAD2"},
			{0x51,"NUMPAD3"},
			{0x52,"NUMPAD0"},
			{0x53,"DECIMAL"},
			{0x56,"OEM_102"},
			{0x57,"F11"},
			{0x58,"F12"},
			{0x64,"F13"},
			{0x65,"F14"},
			{0x66,"F15"},
			{0x70,"KANA"},
			{0x73,"ABNT_C1"},
			{0x79,"CONVERT"},
			{0x7B,"NOCONVERT"},
			{0x7D,"YEN"},
			{0x7E,"ABNT_C2"},
			{0x8D,"NUMPADEQUALS"},
			{0x90,"PREVTRACK"},
			{0x91,"AT"},
			{0x92,"COLON"},
			{0x93,"UNDERLINE"},
			{0x94,"KANJI"},
			{0x95,"STOP"},
			{0x96,"AX"},
			{0x97,"UNLABELED"},
			{0x99,"NEXTTRACK"},
			{0x9C,"NUMPADENTER"},
			{0x9D,"RCONTROL"},
			{0xA0,"MUTE"},
			{0xA1,"CALCULATOR"},
			{0xA2,"PLAYPAUSE"},
			{0xA4,"MEDIASTOP"},
			{0xAE,"VOLUMEDOWN"},
			{0xB0,"VOLUMEUP"},
			{0xB2,"WEBHOME"},
			{0xB3,"NUMPADCOMMA"},
			{0xB5,"DIVIDE"},
			{0xB7,"SYSRQ"},
			{0xB8,"RMENU"},
			{0xC5,"PAUSE"},
			{0xC7,"HOME"},
			{0xC8,"UP"},
			{0xC9,"PRIOR"},
			{0xCB,"LEFT"},
			{0xCD,"RIGHT"},
			{0xCF,"END"},
			{0xD0,"DOWN"},
			{0xD1,"NEXT"},
			{0xD2,"INSERT"},
			{0xD3,"DELETE"},
			{0xDB,"LWIN"},
			{0xDC,"RWIN"},
			{0xDD,"APPS"},
			{0xDE,"POWER"},
			{0xDF,"SLEEP"},
			{0xE3,"WAKE"},
			{0xE5,"WEBSEARCH"},
			{0xE6,"WEBFAVORITES"},
			{0xE7,"WEBREFRESH"},
			{0xE8,"WEBSTOP"},
			{0xE9,"WEBFORWARD"},
			{0xEA,"WEBBACK"},
			{0xEB,"MYCOMPUTER"},
			{0xEC,"MAIL"},
			{0xED,"MEDIASELECT"},
		};

		inline const char* midiNotes[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

		utils::keys::Key ParseKey(unsigned long cbData, void* lpvData, DeviceType type);
		std::string toString(utils::keys::Key key);
		void SaveToConfigFile();
		void LoadConfig(mINI::INIMap<std::string> bindingsConfig);
		bool BindKey(BindingType bindingType, Key key);
	}
}

template <>
struct std::hash<utils::keys::Key>
{
	std::size_t operator()(const utils::keys::Key& k) const
	{
		using std::size_t;
		using std::hash;

		return (hash<unsigned int>()((unsigned int)k.type)
			^ (hash<unsigned int>()(k.value) << 4));
	}
};