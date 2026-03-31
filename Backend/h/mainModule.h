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
	const metricBuffer& getMetricBufferSynth() const { return this->bufferSynth; }
	const metricBuffer& getMetricBufferModel() const { return this->bufferModel; }

	void setSamplesActive(bool value) { this->samplesActive = value; }
	void setSynthActive(bool value) { this->synthActive = value; }
	void setModelActive(bool value) { this->modelActive = value; }

	void startRecording(audioSignal& ref, audioSignal& comp);
	void startRecordingSynth(audioSignal& ref, audioSignal& comp);
	void startRecordingModel(audioSignal& ref, audioSignal& comp);
	void stopRecording(audioSignal& ref, audioSignal& comp);
	void stopRecordingSynth(audioSignal& ref, audioSignal& comp);
	void stopRecordingModel(audioSignal& ref, audioSignal& comp);

	const bool getSamplesActive() const { return samplesActive; }
	const bool getSynthActive() const { return synthActive; }
	const bool getModelActive() const { return modelActive; }

private:
	std::shared_ptr<voices> voiceManager;
	samplesModule samples;
	synthModule synth;
	modelModule model;

	metricBuffer bufferSynth;
	metricBuffer bufferModel;

	bool samplesActive;
	bool synthActive;
	bool modelActive;
};