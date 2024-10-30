#pragma once

namespace hooks {
	namespace midi {
		typedef DWORD(__cdecl* ParseMidiMessage)(DWORD);
		inline ParseMidiMessage oParseMidiMessage;

		void Setup();
	}
}