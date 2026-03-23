#pragma once

#include "module.h"
#include "../lib/miniaudio.h"

#include <map>
#include <queue>

struct LoopingSample {
    ma_data_source_base base;

    float* data;
    ma_uint64 frameCount;
    ma_uint32 channels;
    ma_uint32 sampleRate;

    ma_uint64 cursor;
    ma_uint64 loopStart;
    ma_uint64 loopEnd;
    ma_uint64 fadeLength;

    bool loopActive;
    ma_uint64 fadeCursor;
};

struct sampleSet {
    ma_sound attack;
    ma_sound sustain1;
    ma_sound sustain2;
    ma_sound release;

    LoopingSample looping;

    bool useFirst = true;
    float crossfadePos = 0.0f;
    float crossfadeStep = 1.0f / 4096.0f;

    void startLoop();
    void stopLoop();
    void updateLooping();
};

class samplesModule : public module {
public:
	samplesModule(std::shared_ptr<voices> voiceManager, int maxPolyphony=1024);
	~samplesModule();
	void play(const noteSignal& signal, audioSignal& output) override;
	void initEngine();

	const std::map<std::pair<int, int>, sampleSet*>& getSamples() const { return this->samples; }
private:
	std::map<std::pair<int, int>, sampleSet*> samples;
	ma_engine engine;
    ma_device device;

	int maxPolyphony;
};

