#pragma once

#include "modelModule.h"
#include "synthModule.h"
#include "samplesModule.h"
#include "voices.h"
#include "metrices.h"

class mainModule {
public:
	mainModule();

	void play(noteSignal& MIDISignal);

	std::shared_ptr<voices> getVoicesManager() const { return this->voiceManager; }
	const samplesModule& getSamplesModule() const { return this->samples; }
	const synthModule& getSynthModule() const { return this->synth; }
	const modelModule& getModelModule() const { return this->model; }

	void setSamplesActive(bool value) { this->samplesActive = value; }
	void setSynthActive(bool value) { this->synthActive = value; }
	void setModelActive(bool value) { this->modelActive = value; }

	const bool getSamplesActive() const { return samplesActive; }
	const bool getSynthActive() const { return synthActive; }
	const bool getModelActive() const { return modelActive; }

private:
	std::shared_ptr<voices> voiceManager;
	samplesModule samples;
	synthModule synth;
	modelModule model;

	metric1 m1;
	metric2 m2;

	bool samplesActive;
	bool synthActive;
	bool modelActive;
};