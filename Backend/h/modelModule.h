#pragma once

#include <vector>
#include "module.h"
#include "voices.h"

struct modelPipeParams {
	modelVoiceParams baseParams;

	modelPipeParams() : baseParams() {

	}
};

class modelPipe {
public:
	modelPipe();

	void load(modelPipeParams& params);
	void noteOn();
	void noteOff();
	float process();

	modelPipeParams& getParams() { return this->params; }

private:
	modelPipeParams params;
	bool playing;
};

class modelVoice {
public:
	modelVoice();

	void load(modelVoiceParams& params);
	void noteOn(int note);
	void noteOff(int note);
	float process();

	modelPipeParams pipeParams(int note);

	std::vector<modelPipe>& getPipes() { return this->pipes; }
	modelVoiceParams& getParams() { return this->params; }
private:
	std::vector<modelPipe> pipes;
	modelVoiceParams params;
};

class modelModule : public module {
public:
	modelModule(std::shared_ptr<voices> voiceManager);
	
	void play(const noteSignal& signal) override;
	void processSample(float& outL, float& outR) override;
	void load() override;
	void unload() override;

private:
	std::vector<modelVoice> allVoices;
};