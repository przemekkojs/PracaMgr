#pragma once

#include <map>

#include "module.h"
#include "voices.h"

struct onePole {
    float a = 0.0f;
    float z = 0.0f;

    void setLowpass(float cutoff) { a = cutoff; }

    float process(float x) {
        z = (1.0f - a) * x + a * z;
        return z;
    }
};

struct synthVoice {
    bool playing = false;
    bool phaseReset = false;
    float freq = 440.0f;
    float feedback = 0.0f;
    float env = 0.0f;
    float jetEnv = 0.0f;
    float sampleRate = module::SAMPLE_RATE;
    float lastPipe = 0.0f;

    signalsmith::delay::Delay<float> pipeDelay;
    signalsmith::delay::Delay<float> jetDelay;
    

    onePole lossFilter;
    pipeParams params;

    synthVoice(const pipeParams& p);

    void noteOn(int midiNote);
    float process();

    void noteOff() { this->playing = false; }
    inline float noise() { return (rand() / (float)RAND_MAX) * 2.0f - 1.0f; }
    inline float nonlinearity(float x) { return x - x * x * x; }
};

class synthModule : public module {
public:
	synthModule(std::shared_ptr<voices> voiceManager);

	void play(const noteSignal& signal) override;
	void processSample(float& outL, float& outR) override;
	void load() override;
	void unload() override;

private:    
	std::map<int, synthVoice> allVoices;
};