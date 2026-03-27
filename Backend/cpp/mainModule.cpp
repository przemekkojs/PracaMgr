#include "../h/mainModule.h"

#include <iostream>

mainModule::mainModule() :
	voiceManager(std::make_shared<voices>()),
	samples(voiceManager),
	synth(voiceManager),
	model(voiceManager) {
	std::cout << "Main module initialized." << std::endl;
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

	// Tutaj jakoœ ten sygna³ trzeba bêdzie odtwarzaæ
}