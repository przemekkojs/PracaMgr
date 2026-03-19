#pragma once

#include "modelModule.h"
#include "synthModule.h"
#include "samplesModule.h"
#include "voices.h"

class mainModule {
public:
	mainModule();

	void play(noteSignal& MIDISignal);

	std::shared_ptr<voices> getVoicesManager() const { return this->voiceManager; }
	samplesModule getSamplesModule() const { return this->samples; }
	synthModule getSynthModule() const { return this->synth; }
	modelModule getModelModule() const { return this->model; }

	void setSamplesActive(bool value) { this->samplesActive = value; }
	void setSynthActive(bool value) { this->synthActive = value; }
	void setModelActive(bool value) { this->modelActive = value; }

	bool getSamplesActive() { return samplesActive; }
	bool getSynthActive() { return synthActive; }
	bool getModelActive() { return modelActive; }

private:
	std::shared_ptr<voices> voiceManager;
	samplesModule samples;
	synthModule synth;
	modelModule model;

	bool samplesActive;
	bool synthActive;
	bool modelActive;
};