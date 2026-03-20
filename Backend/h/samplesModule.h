#pragma once

#include "module.h"
#include "../lib/miniaudio.h"

#include <map>

struct sampleSet {
	ma_sound attack;
	ma_sound sustain;
	ma_sound release;
};

class samplesModule : public module {
public:
	samplesModule(std::shared_ptr<voices> voiceManager);
	~samplesModule();
	void play(const noteSignal& signal, audioSignal& output) override;
	void initEngine();

	const std::map<std::pair<int, int>, sampleSet*>& getSamples() const { return this->samples; }
private:
	std::map<std::pair<int, int>, sampleSet*> samples;
	ma_engine engine;
};

