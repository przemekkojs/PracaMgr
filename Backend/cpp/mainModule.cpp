#include "../h/mainModule.h"

#include <iostream>

mainModule::mainModule() : samples(this->voiceManager), synth(this->voiceManager), model(this->voiceManager) {
	this->voiceManager = std::make_shared<voices>();

	this->samplesActive = false;
	this->modelActive = false;
	this->synthActive = false;
}

void mainModule::play(noteSignal& MIDISignal) {
	std::cout << MIDISignal.toString() << std::endl;

	audioSignal synthSignal;
	audioSignal modelSignal;
	audioSignal samplesSignal;

	if (this->synthActive)
		this->synth.play(MIDISignal, synthSignal);

	if (this->modelActive)
		this->model.play(MIDISignal, modelSignal);

	if (this->samplesActive)
		this->samples.play(MIDISignal, samplesSignal);

	// Tutaj jakoœ ten sygna³ trzeba bêdzie odtwarzaæ
}