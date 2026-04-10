#pragma once

#include <map>
#include <cmath>
#include <random>

#include "module.h"
#include "voices.h"


struct synthVoice {
    bool playing = false;
    int note = -1;

    float freq = 0.0f;
    float phase = 0.0f;
    float sampleRate = 48000.0f;

    // envelope
    float env = 0.0f;
    float attack = 0.001f;   // szybki start
    float release = 0.0005f; // ³agodne wybrzmiewanie

    // noise (oddech piszcza³ki)
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist{ -1.0f, 1.0f };

    synthVoice() {
        rng.seed(std::random_device{}());
    }

    inline float midiToFreq(int note) {
        return 440.0f * std::pow(2.0f, (note - 69) / 12.0f);
    }

    void noteOn(int n) {
        note = n;
        freq = midiToFreq(n);
        playing = true;
    }

    void noteOff() {
        playing = false;
    }

    float process() {
        if (env < 0.0001f && !playing) return 0.0f;

        // envelope
        if (playing) {
            env += attack;
            if (env > 1.0f) env = 1.0f;
        }
        else {
            env -= release;
            if (env < 0.0f) env = 0.0f;
        }

        // oscillator (lekko "organowy")
        float sample =
            std::sin(phase) * 0.8f +
            std::sin(2.0f * phase) * 0.15f +
            std::sin(3.0f * phase) * 0.05f;

        // noise (tylko na pocz¹tku)
        float noise = dist(rng) * 0.02f * env;

        // update phase
        phase += 2.0f * M_PI * freq / sampleRate;
        if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;

        return (sample + noise) * env;
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
	std::map<int, synthVoice> allVoices;
};