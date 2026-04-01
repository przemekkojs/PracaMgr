#include "../h/modelModule.h"

#include <iostream>

modelModule::modelModule(std::shared_ptr<voices> voiceManager) : module(std::move(voiceManager)) {

}

void modelModule::play(const noteSignal& signal) {

}

void modelModule::processSample(float& outL, float& outR) {

}

void modelModule::load() {

}

void modelModule::unload() {

}