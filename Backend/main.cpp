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

	instrument.setModelActive(true);
	instrument.setSamplesActive(true);
	instrument.setSynthActive(true);
	instrument.getVoicesManager()->setActive(1, true);

	while (true) {
		noteSignal signal = instrument.getSignal();

		if (signal == EMPTY_NOTE_SIGNAL)
			continue;

		instrument.play(signal);
	}

	return 0;
}