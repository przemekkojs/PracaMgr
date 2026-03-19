#pragma once

#include "signal.h"
#include "voices.h"

#include <memory>

class module {
public:
	explicit module(std::shared_ptr<voices> voiceManager);
	virtual ~module() = default;

	virtual void play(const noteSignal& signal, audioSignal& output) = 0;

protected:
	std::shared_ptr<voices> voiceManager;
};