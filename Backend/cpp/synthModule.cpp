#include "../h/synthModule.h"

synthModule::synthModule(std::shared_ptr<voices> voiceManager) : module(std::move(voiceManager)), allVoices() { }

void synthModule::play(const noteSignal& signal) {
    std::vector<voice> activeVoices = this->voiceManager->getActiveVoices();

    for (int i = 0; i < activeVoices.size(); i++) {
        voice& v = activeVoices[i];
        int id = v.getId();

        synthVoice& sV = this->allVoices[id - 1];
        signal.on ? sV.noteOn(signal.note) : sV.noteOff(signal.note);
    }
}

void synthModule::processSample(float& outL, float& outR) {
    float sum = 0.0f;

    for (auto& v : allVoices) {
        sum += v.process();
    }

    outL += sum;
    outR += sum;
}

void synthModule::load() {
    for (auto& v : this->voiceManager->getVoices()) {
        synthVoiceParams& params = v.getSynthParams();
        synthVoice sV;
        voiceType vT = v.getVoiceType();

        sV.load(params, vT);
        this->allVoices.push_back(std::move(sV));
    }
}

void synthModule::unload() {
    this->allVoices.clear();
}
