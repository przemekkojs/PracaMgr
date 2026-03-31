#include "../h/mainModule.h"

#include <iostream>

mainModule::mainModule() :
	voiceManager(std::make_shared<voices>()),
	samples(voiceManager),
	synth(voiceManager),
	model(voiceManager) {
	std::cout << "Initializing main module" << std::endl;

	unsigned int ports = midiIn.getPortCount();

	if (ports == 0)
		throw new std::exception("No devices active");

	for (unsigned int portIndex = 0; portIndex < ports; portIndex++) {
		std::cout << portIndex << " " << midiIn.getPortName(portIndex) << std::endl;
	}

	int portIndex = 1;
	midiIn.openPort(portIndex);

	std::cout << "Main module initialized." << std::endl;
}

noteSignal mainModule::getSignal() {
	std::vector<unsigned char> message;
	double timestamp = midiIn.getMessage(&message);

	if (message.size() >= 3) {
		unsigned char note = message[1];
		unsigned char channel = message[0] & 0x0F;
		unsigned char on = (message[0] & 0xF0) == 0x90;

		return noteSignal(note, channel, on);
	}

	return EMPTY_NOTE_SIGNAL;
}

void mainModule::play(noteSignal& MIDISignal) {
	std::cout << MIDISignal.toString() << std::endl;

	audioSignal synthSignal;
	audioSignal modelSignal;
	audioSignal samplesSignal;

	if (this->getSynthActive())
		this->synth.play(MIDISignal, synthSignal);

	if (this->getModelActive())
		this->model.play(MIDISignal, modelSignal);

	if (this->getSamplesActive())
		this->samples.play(MIDISignal, samplesSignal);

	// Tutaj jakoś ten sygnał trzeba będzie odtwarzać

	this->bufferSynth.push(samplesSignal, synthSignal);
	this->bufferModel.push(samplesSignal, modelSignal);
}
