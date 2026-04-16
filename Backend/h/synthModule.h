#pragma once

#include <map>

#include "module.h"
#include "voices.h"

#include <vector>
#include <cmath>
#include <random>

enum class AdsrState { IDLE, ATTACK, DECAY, SUSTAIN, RELEASE };

class ADSR {
public:	
	float process();
	void noteOn() { state = AdsrState::ATTACK; value = 0.0f; }
	void noteOff() { state = AdsrState::RELEASE; }
	bool isActive() const { return state != AdsrState::IDLE; }

	void setAttack(float seconds) { this->calculateRate(this->attackRate, seconds); }
	void setDecay(float seconds) { this->calculateRate(this->decayRate, seconds); }
	void setRelease(float seconds) { this->calculateRate(this->releaseRate, seconds); }
	void setSustain(float seconds) { this->calculateRate(this->sustainLevel, seconds); }

	void calculateRate(float& what, float seconds) const;

private:
	AdsrState state = AdsrState::IDLE;
	float value = 0.0f;
	float sampleRate = 48000.0f;
	float attackRate = 0.001f;
	float decayRate = 0.0005f;
	float releaseRate = 0.0008f;
	float sustainLevel = 0.7f;
};

struct synthPipeParams {
	synthVoiceParams baseParams;

	float frequency;
	float delaySamples;
	float jetDelaySamples;

	synthPipeParams() : baseParams() {
		this->frequency = 440.0f;
		this->delaySamples = 100;
		this->jetDelaySamples = 20;
	}

	std::string toString() const {
		std::string result;

		result += "[baseParams]\n" + baseParams.toString() + "\n";
		result += "frequency=" + std::to_string(frequency) + "\n";
		result += "delaySamples=" + std::to_string(delaySamples) + "\n";
		result += "jetDelaySamples=" + std::to_string(jetDelaySamples);

		return result;
	}
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
	float nonlinear(float x, float env);

	float pinkNoise();
	float whiteNoise();
	float brownNoise();

	synthPipeParams& getParams() { return this->params; }

private:
	synthPipeParams params;

	std::vector<float> delayLine;
	std::vector<float> jetDelayLine;

	int writeIdx = 0;
	int jetIdx = 0;

	float lastSample = 0.0f;
	float lossState = 0.0f;
	float loopLP = 0.0f;
	float jetLP = 0.0f;
	float smoothBreath = 0.0f;
	float smoothedNoise = 0.0f;
	float windDrift = 0.0f;
	float lastPipeOut = 0.0f;

	std::mt19937 rng;
	std::uniform_real_distribution<float> noise{ -1.0f, 1.0f };
	ADSR adsr;
};


class synthVoice {
public:
	synthVoice();

	void load(synthVoiceParams& params);
	void noteOn(int note);
	void noteOff(int note);
	float process();

	synthPipeParams pipeParams(int note);

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