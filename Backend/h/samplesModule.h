#pragma once

#include "module.h"

class samplesModule : module {
public:
	samplesModule(std::shared_ptr<voices> voiceManager);

	audioSignal& play(noteSignal& signal) override;
};