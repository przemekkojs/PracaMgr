#include "h/mainModule.h"
#include "lib/RtMidi.h"
#include "h/signal.h"

#include <vector>
#include <iostream>

// To będzie klasa tylko na TESTY
// Uruchamianie będzie z poziomu
// pythona, po połączeniu modułów

int main()
{
	mainModule instrument;
	RtMidiIn midiIn;

	unsigned int ports = midiIn.getPortCount();
	
	if (ports == 0)
		return -1;

	for (unsigned int portIndex = 0; portIndex < ports; portIndex++) {
		std::cout << portIndex << " " << midiIn.getPortName(portIndex) << std::endl;
	}

	int portIndex = 0;
	std::cout << "Port index: ";
	std::cin >> portIndex;

	midiIn.openPort(portIndex);

	while (true) {
		std::vector<unsigned char> message;
		double timestamp = midiIn.getMessage(&message);

		noteSignal MIDISignal = message.size() < 3 ?
			noteSignal(128, 0, false) :
			noteSignal(message[1], message[2], message[0] == 0x08);

		if (MIDISignal.note > 127)
			continue;

		std::cout << (int)MIDISignal.note << " " 
				  << (int)MIDISignal.channel << " "
				  << (int)MIDISignal.on << std::endl;
	}	

	return 0;
}