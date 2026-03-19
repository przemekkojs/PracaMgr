#pragma once

#include "module.h"

class synthModule : module {
public:
	synthModule(std::shared_ptr<voices> voiceManager);

	audioSignal& play(noteSignal& signal) override;
};