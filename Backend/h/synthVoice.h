#pragma once

#include "voices.h"
#include "synthPipe.h"
#include "config.h"

#include <memory>
#include <vector>

class synthVoice {
public:
	synthVoice();

	synthVoice(const synthVoice&) = delete;
	synthVoice& operator=(const synthVoice&) = delete;
	synthVoice(synthVoice&&) = default;
	synthVoice& operator=(synthVoice&&) = default;

	void load(const synthVoiceParams& params, voiceType vT, bool isModel=false);
	void noteOn(int note);
	void noteOff(int note);
	float process();

	synthPipeParams pipeParams(int note) const;

	std::vector<std::unique_ptr<synthPipe>>& getPipes() { return this->pipes; }
	synthVoiceParams& getParams() { return this->params; }

private:
	std::vector<std::unique_ptr<synthPipe>> pipes;
	synthVoiceParams params;
};
