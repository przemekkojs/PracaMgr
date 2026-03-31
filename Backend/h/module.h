#pragma once

#include "signal.h"
#include "voices.h"

#include <memory>

class module {
public:
	explicit module(std::shared_ptr<voices> voiceManager);
	virtual ~module() = default;

	virtual void play(const noteSignal& signal) = 0;
	virtual void processSample(float& outL, float& outR) = 0;
	virtual void unload() = 0;
	virtual void load() = 0;

	const bool isActive() const { return active; }
	void setActive(bool value);

protected:
	std::shared_ptr<voices> voiceManager;
	bool active;
};