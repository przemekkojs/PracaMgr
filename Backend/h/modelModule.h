#pragma once

#include "module.h"

class modelModule : public module {
public:
	modelModule(std::shared_ptr<voices> voiceManager);
	
	void play(const noteSignal& signal, audioSignal& output) override;
};