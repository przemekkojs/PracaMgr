#include "../h/synthModule.h"

#include <iostream>

synthModule::synthModule(std::shared_ptr<voices> voiceManager) : module(std::move(voiceManager)), allVoices() {

}

void synthModule::play(const noteSignal& signal) {
    if (signal.on) {
        auto& voice = allVoices[signal.note];
        voice.noteOn(signal.note);
    }
    else {
        auto it = allVoices.find(signal.note);
        if (it != allVoices.end()) {
            it->second.noteOff();
        }
    }
}

void synthModule::processSample(float& outL, float& outR) {
    float sum = 0.0f;

    for (auto& [note, voice] : allVoices) {
        sum += voice.process();
    }

    outL += sum * 0.2f;
    outR += sum * 0.2f;
}

void synthModule::load() {
    /*for (auto& voice : this->voiceManager->getVoices()) {
        pipeParams& params = voice.getSynthParams();
        this->allVoices.emplace(voice.getId(), synthVoice(params));

        std::cout << "Loaded " << voice.getName() << std::endl;
    }*/
}

void synthModule::unload() {
    this->allVoices.clear();
}


static inline float midiToFreq(int midi) {
    return 440.0f * std::pow(2.0f, (midi - 69) / 12.0f);
}
