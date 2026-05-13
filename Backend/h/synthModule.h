#pragma once

#include "module.h"
#include "voices.h"
#include "config.h"
#include "synthPipe.h"
#include "synthVoice.h"

#include <vector>

class synthModule : public module {
public:
	synthModule(std::shared_ptr<voices> voiceManager, bool isModel=false);

	void play(const noteSignal& signal) override;
	void processSample(float& outL, float& outR) override;
	void load() override;
	void unload() override;

private:
	std::vector<synthVoice> allVoices;
	bool isModel = false;
};