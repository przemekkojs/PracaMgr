#include "../h/synthModule.h"

#include <iostream>

synthModule::synthModule(std::shared_ptr<voices> voiceManager) : module(std::move(voiceManager)) {

}

void synthModule::play(const noteSignal& signal) {

}

void synthModule::processSample(float& outL, float& outR) {

}

void synthModule::load() {

}

void synthModule::unload() {

}