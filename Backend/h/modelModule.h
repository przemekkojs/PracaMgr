#pragma once

#include "module.h"

class modelModule : module {
public:
	modelModule(std::shared_ptr<voices> voiceManager);
	
	audioSignal& play(noteSignal& signal) override;
};