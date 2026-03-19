#pragma once

#include "signal.h"
#include "voices.h"

#include <memory>

class module {
public:
	module(std::shared_ptr<voices> voiceManager);

	virtual audioSignal& play(noteSignal& signal) = 0;

private:
	std::shared_ptr<voices> voiceManager;
};