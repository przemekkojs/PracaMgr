#pragma once

#include "module.h"
#include "../lib/miniaudio.h"

#include <map>
#include <queue>
#include <iostream>
#include <algorithm>

class samplesModule : public module {
public:
	samplesModule(std::shared_ptr<voices> voiceManager, int maxPolyphony=1024);
	~samplesModule();
	void play(const noteSignal& signal, audioSignal& output) override;
	void initEngine();

	const std::map<std::pair<int, int>, int*>& getSamples() const { return this->samples; }
private:
	std::map<std::pair<int, int>, int*> samples;
	ma_engine engine;
    ma_device device;

	int maxPolyphony;
};

