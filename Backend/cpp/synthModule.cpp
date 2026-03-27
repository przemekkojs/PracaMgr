#include "../h/synthModule.h"

#include <iostream>

synthModule::synthModule(std::shared_ptr<voices> voiceManager) : module(std::move(voiceManager)) {
	std::cout << "Synth module init" << std::endl;
}

void synthModule::play(const noteSignal& signal, audioSignal& output) {
	std::cout << "Synth signal" << std::endl;
}

void synthModule::load() {

}

void synthModule::unload() {

}