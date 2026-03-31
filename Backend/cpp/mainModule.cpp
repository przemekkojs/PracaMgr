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

	
}

void mainModule::startRecording(audioSignal& ref, audioSignal& comp) {
	this->startRecordingModel(ref, comp);
	this->startRecordingSynth(ref, comp);
}

void mainModule::startRecordingSynth(audioSignal& ref, audioSignal& comp) {
	this->bufferSynth.record(ref, comp);
}

void mainModule::startRecordingModel(audioSignal& ref, audioSignal& comp) {
	this->bufferModel.record(ref, comp);
}

void mainModule::stopRecording(audioSignal& ref, audioSignal& comp) {
	this->stopRecordingModel(ref, comp);
	this->stopRecordingSynth(ref, comp);
}

void mainModule::stopRecordingSynth(audioSignal& ref, audioSignal& comp) {
	this->bufferSynth.stopRecording(ref, comp);
}

void mainModule::stopRecordingModel(audioSignal& ref, audioSignal& comp) {
	this->bufferModel.stopRecording(ref, comp);
}