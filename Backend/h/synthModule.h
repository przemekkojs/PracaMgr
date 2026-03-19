#pragma once

#include "module.h"

class synthModule : public module {
public:
	synthModule(std::shared_ptr<voices> voiceManager) : module(std::move(voiceManager)) { }

	void play(const noteSignal& signal, audioSignal& output) override;
};