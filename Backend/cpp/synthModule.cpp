#include "../h/synthModule.h"

#include <iostream>

synthModule::synthModule(std::shared_ptr<voices> voiceManager) : module(std::move(voiceManager)) {
	std::cout << "Synth module init" << std::endl;
}

void synthModule::play(const noteSignal& signal) {
	// std::cout << "Synth signal" << std::endl;
}

void synthModule::processSample(float& outL, float& outR) {

}

void synthModule::load() {

}

void synthModule::unload() {

}