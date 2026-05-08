#pragma once

#include "adsr.h"
#include "voices.h"

#include <random>
#include <string>
#include <cmath>
#include <memory>

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
};

class synthPipe {
public:
	synthPipe();
	virtual ~synthPipe() = default;

	void load(synthPipeParams& params);
	void noteOn();
	void noteOff();
	virtual float process();

	float lossFilter(float x);
	float lowpass(float x);
	float jetLowpass(float x);
	float nonlinear(float x, float env);
	float allpass(float x);

	float pinkNoise();
	float whiteNoise();
	float brownNoise();
	float fastNoise();


	void writePipe(float input);
	bool isActive();

	float processEnvelope();
	float readPipe();
	float processPipeFilter(float pipeOut);

	virtual float computeBreath(float env, float pipeOut);
	virtual float processJet(float breath, float pipeOut);
	virtual float processExcitation(float jet, float env);
	virtual float processFeedback(float excitation, float pipeOut);
	virtual float processOutput(float pipeOut);

	synthPipeParams& getParams() { return this->params; }

protected:
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

	float ap_x1 = 0.0f;
	float ap_y1 = 0.0f;
	float jetState = 0.0f;
	float loss_x1 = 0.0f;
	float loss_y1 = 0.0f;
	float reflectionLP = 0.0f;

	std::mt19937 rng;
	uint32_t state = 123456789;
	std::uniform_real_distribution<float> noise{ -1.0f, 1.0f };
	ADSR adsr;
};

class flutePipe : public synthPipe {
public:
	flutePipe() : synthPipe() {}

private:
};

class stringPipe : public synthPipe {
public:
	stringPipe() : synthPipe() {}

private:
};

class principalPipe : public synthPipe {
public:
	principalPipe() : synthPipe() {}

private:
};

class reedPipe : public synthPipe {
public:
	reedPipe() : synthPipe() {}

	float computeBreath(float env, float pipeOut) override;
	float processJet(float breath, float pipeOut) override;
	float processExcitation(float deltaP, float env) override;
	float processFeedback(float flow, float pipeOut) override;

private:
	float lastOpening = 0.0f;
	float resonance_z1 = 0.0f;
	float resonance_z2 = 0.0f;
};

class flutePipeModel : public synthPipe {
public:
	flutePipeModel() : synthPipe() {}

private:
};

class stringPipeModel : public synthPipe {
public:
	stringPipeModel() : synthPipe() {}

private:
};

class principalPipeModel : public synthPipe {
public:
	principalPipeModel() : synthPipe() {}

private:
};

class reedPipeModel : public synthPipe {
public:
	reedPipeModel() : synthPipe() {}

private:
};
