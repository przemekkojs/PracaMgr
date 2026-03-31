#include "../h/mainModule.h"

#include <iostream>

mainModule::mainModule() :
	voiceManager(std::make_shared<voices>()),
	samples(voiceManager),
	synth(voiceManager),
	model(voiceManager),
	samplesActive(false),
	synthActive(false),
	modelActive(false)
{
	std::cout << "Main module initialized." << std::endl;
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

	this->bufferSynth.push(samplesSignal, synthSignal);
	this->bufferModel.push(samplesSignal, modelSignal);
}
