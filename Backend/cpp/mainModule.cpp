#include "../h/mainModule.h"

#include <iostream>

mainModule::mainModule() :
	voiceManager(std::make_shared<voices>()),
	samples(voiceManager),
	synth(voiceManager),
	model(voiceManager) {
	std::cout << "Initializing main module" << std::endl;

	this->modelActive = false;
	this->synthActive = false;
	this->samplesActive = false;

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

	bool synthActive = this->getSynthActive();
	bool modelActive = this->getModelActive();
	bool samplesActive = this->getSamplesActive();

	if (MIDISignal.on) {
		if (samplesActive) {
			if (modelActive) {
				this->bufferModel.clear();
				this->bufferModel.start();
			}

			if (synthActive) {
				this->bufferSynth.clear();
				this->bufferSynth.start();
			}
		}
	}
	else {
		this->bufferModel.stop();
		this->bufferSynth.stop();

		std::cout << this->bufferModel.getRefBuffer().size() << " " << this->bufferModel.getCompBuffer().size() << std::endl;
		std::cout << this->bufferSynth.getRefBuffer().size() << " " << this->bufferSynth.getCompBuffer().size() << std::endl;
	}

	audioSignal synthSignal;
	audioSignal modelSignal;
	audioSignal samplesSignal;

	if (synthActive)
		this->synth.play(MIDISignal, synthSignal);

	if (modelActive)
		this->model.play(MIDISignal, modelSignal);

	if (samplesActive)
		this->samples.play(MIDISignal, samplesSignal);

	this->bufferSynth.push(samplesSignal, synthSignal);
	this->bufferModel.push(samplesSignal, modelSignal);
}
