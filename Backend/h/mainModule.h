#pragma once

#include "../lib/RtMidi.h"

#include "modelModule.h"
#include "synthModule.h"
#include "samplesModule.h"
#include "voices.h"
#include "metrices.h"

class mainModule {
public:
	mainModule();
	~mainModule();

	void play(noteSignal& MIDISignal);
	noteSignal getSignal();

	std::shared_ptr<voices> getVoicesManager() const { return this->voiceManager; }
	samplesModule& getSamplesModule() { return this->samples; }
	synthModule& getSynthModule() { return this->synth; }
	modelModule& getModelModule() { return this->model; }

	void setSamplesActive(bool value) { this->setModuleActive(value, this->samples); }
	void setSynthActive(bool value) { this->setModuleActive(value, this->synth); }
	void setModelActive(bool value) { this->setModuleActive(value, this->model); }
	void setModuleActive(bool value, module& m) { m.setActive(value); }
	void processSample(float& outL, float& outR);
	static void audioCallback(ma_device* device, void* output, const void* input, ma_uint32 frameCount);
	void initDevice();
	void initEngine();

	const bool getSamplesActive() const { return this->samples.isActive(); }
	const bool getSynthActive() const { return this->synth.isActive(); }
	const bool getModelActive() const { return this->model.isActive(); }

private:
	std::shared_ptr<voices> voiceManager;
	samplesModule samples;
	synthModule synth;
	modelModule model;
	RtMidiIn midiIn;

	ma_engine engine;
	ma_device device;

	metricBuffer bufferSynth;
	metricBuffer bufferModel;
};