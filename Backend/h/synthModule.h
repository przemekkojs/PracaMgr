#pragma once

#include "module.h"
// Biblioteka "Signalsmith DSP" - niby header-only

class synthModule : public module {
public:
	synthModule(std::shared_ptr<voices> voiceManager);

	void play(const noteSignal& signal, audioSignal& output) override;
	void load() override;
	void unload() override;
};