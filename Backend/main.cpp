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

	int portIndex = 1;
	midiIn.openPort(portIndex);

	instrument.setModelActive(false);
	instrument.setSamplesActive(true);
	instrument.setSynthActive(false);
	instrument.getVoicesManager()->setActive(1, true);	

	while (true) {
		std::vector<unsigned char> message;
		double timestamp = midiIn.getMessage(&message);

		if (message.size() >= 3) {
			unsigned char note = message[1];
			unsigned char channel = message[0] & 0x0F;
			unsigned char on = (message[0] & 0xF0) == 0x90;
			noteSignal MIDISignal = noteSignal(note, channel, on);

			instrument.play(MIDISignal);
		}				
	}	

	return 0;
}