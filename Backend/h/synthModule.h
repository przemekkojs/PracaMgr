#pragma once

#include <map>

#include "module.h"
#include "voices.h"

#include <vector>
#include <cmath>
#include <random>

struct synthPipeParams {
	float frequency;
	float sampleRate;
	float excitationGain;
	float reflection;
	float loss;
	float jetGain;
	float noiseGain;

	int delaySamples;      // d³ugoœæ rury
	int jetDelaySamples;   // opóŸnienie strugi (KRUCIAL)

	synthPipeParams() {}
};

class synthPipe {
public:
	synthPipe();

	void load(synthPipeParams& params);
	void noteOn();
	void noteOff();
	float process();
	float lossFilter(float x);
	float lowpass(float x);
	float jetLowpass(float x);
	float nonlinear(float x);

	float pinkNoise();
	float whiteNoise();
	float brownNoise();

	synthPipeParams& getParams() { return this->params; }

private:
	bool playing;
	synthPipeParams params;

	std::vector<float> delayLine;
	std::vector<float> jetDelayLine;

	int writeIdx = 0;
	int jetIdx = 0;

	float lastSample = 0.0f;
	float lossState = 0.0f;
	float loopLP = 0.0f;
	float jetLP = 0.0f;

	std::mt19937 rng;
	std::uniform_real_distribution<float> noise{ -1.0f, 1.0f };
};


class synthVoice {
public:
	synthVoice();

	void load(synthVoiceParams& params);
	void noteOn(int note);
	void noteOff(int note);
	float process();

	synthPipeParams& pipeParams(int note);

	std::vector<synthPipe>& getPipes() { return this->pipes; }
	synthVoiceParams& getParams() { return this->params; }

private:
	std::vector<synthPipe> pipes;
	synthVoiceParams params;
};


class synthModule : public module {
public:
	synthModule(std::shared_ptr<voices> voiceManager);

	void play(const noteSignal& signal) override;
	void processSample(float& outL, float& outR) override;
	void load() override;
	void unload() override;

private:
	std::vector<synthVoice> allVoices;
};