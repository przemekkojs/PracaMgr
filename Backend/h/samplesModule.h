#pragma once
#define MINIAUDIO_IMPLEMENTATION

#include "module.h"
#include "../lib/miniaudio.h"

#include <map>

class samplesModule : public module {
public:
	samplesModule(std::shared_ptr<voices> voiceManager);
	void play(const noteSignal& signal, audioSignal& output) override;

	const std::map<std::pair<int, int>, std::string>& getSamples() const { return this->samples; }
private:
	std::map<std::pair<int, int>, std::string> samples;
};

struct SampleSet {
	ma_sound attack;
	ma_sound sustain;
	ma_sound release;
};