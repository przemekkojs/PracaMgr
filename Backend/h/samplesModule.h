#pragma once

#include "module.h"

class samplesModule : public module {
public:
	samplesModule(std::shared_ptr<voices> voiceManager) : module(std::move(voiceManager)) {}

	void play(const noteSignal& signal, audioSignal& output) override;
};