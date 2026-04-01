#pragma once

#include "module.h"

class modelModule : public module {
public:
	modelModule(std::shared_ptr<voices> voiceManager);
	
	void play(const noteSignal& signal) override;
	void processSample(float& outL, float& outR) override;
	void load() override;
	void unload() override;
};