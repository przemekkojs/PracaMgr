#pragma once

#include "modelModule.h"
#include "synthModule.h"
#include "samplesModule.h"
#include "voices.h"

class mainModule {
public:
	mainModule();

	void play(noteSignal& MIDISignal);

	void setSamplesActive(bool value) { this->samplesActive = value; }
	void setSynthActive(bool value) { this->synthActive = value; }
	void setModelActive(bool value) { this->modelActive = value; }

private:
	std::shared_ptr<voices> voiceManager;
	samplesModule samples;
	synthModule synth;
	modelModule model;

	bool samplesActive;
	bool synthActive;
	bool modelActive;
};