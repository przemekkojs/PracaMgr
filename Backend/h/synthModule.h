#pragma once

#include "module.h"

struct SynthVoice {
    bool active = false;
    float freq = 440.0f;
    float phase = 0.0f;

    void noteOn(int midiNote) {
        freq = 440.0f * powf(2.0f, (midiNote - 69) / 12.0f);
        active = true;
    }

    void noteOff() {
        active = false;
    }

    float process(float sampleRate) {
        if (!active) return 0.0f;

        float sample = sinf(phase * 2.0f * 3.14159265f);

        phase += freq / sampleRate;
        if (phase >= 1.0f) phase -= 1.0f;

        return sample * 0.2f;
    }
};

class synthModule : public module {
public:
	synthModule(std::shared_ptr<voices> voiceManager);

	void play(const noteSignal& signal) override;
	void processSample(float& outL, float& outR) override;
	void load() override;
	void unload() override;

private:
    static constexpr int MAX_VOICES = 16;
    static constexpr float SAMPLE_RATE = 48000.0f;
    SynthVoice voicesArr[MAX_VOICES];    
};