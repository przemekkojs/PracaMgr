#include "../h/mainModule.h"

mainModule::mainModule() : samples(this->voiceManager), synth(this->voiceManager), model(this->voiceManager) {
	this->voiceManager = std::make_shared<voices>();

	this->samplesActive = false;
	this->modelActive = false;
	this->synthActive = false;
}

void mainModule::play(noteSignal& signal) {
	audioSignal synthSignal = synthActive ? this->synth.play(signal) : EMPTY_AUDIO_SIGNAL;
	audioSignal modelSignal = modelActive ? this->model.play(signal) : EMPTY_AUDIO_SIGNAL;
	audioSignal samplesSignal = samplesActive ? this->samples.play(signal) : EMPTY_AUDIO_SIGNAL;

	// Tutaj jakoœ ten sygna³ trzeba bêdzie odtwarzaæ
}