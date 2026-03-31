#include "../h/modelModule.h"

#include <iostream>

modelModule::modelModule(std::shared_ptr<voices> voiceManager) : module(std::move(voiceManager)) {
	std::cout << "Model module init" << std::endl;
}

void modelModule::play(const noteSignal& signal) {
	// std::cout << "Model signal" << std::endl;
}

void modelModule::processSample(float& outL, float& outR) {

}

void modelModule::load() {

}

void modelModule::unload() {

}