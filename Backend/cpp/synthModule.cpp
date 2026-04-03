#include "../h/synthModule.h"

#include <iostream>

synthModule::synthModule(std::shared_ptr<voices> voiceManager) : module(std::move(voiceManager)) {

}

void synthModule::play(const noteSignal& signal) {
    if (signal.on) {
        for (auto& v : voicesArr) {
            if (!v.active) {
                v.noteOn(signal.note);
                break;
            }
        }
    }
    else {
        for (auto& v : voicesArr) {
            if (v.active) {
                v.noteOff();
                break;
            }
        }
    }
}

void synthModule::processSample(float& outL, float& outR) {
    float mix = 0.0f;

    for (auto& v : voicesArr) {
        mix += v.process(SAMPLE_RATE);
    }

    outL = mix;
    outR = mix;
}

void synthModule::load() {

}

void synthModule::unload() {

}