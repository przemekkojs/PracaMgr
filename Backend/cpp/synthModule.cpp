#include "../h/synthModule.h"

#include <iostream>

void synthModule::play(const noteSignal& signal, audioSignal& output) {
	std::cout << "Synth signal" << std::endl;

	std::vector<std::string> activeSamplePaths = voiceManager.get()->getActiveSamplesPaths();
}