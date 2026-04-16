#include "../h/modelModule.h"

#include <iostream>

modelModule::modelModule(std::shared_ptr<voices> voiceManager) : module(std::move(voiceManager)) {

}

void modelModule::play(const noteSignal& signal) {
    std::vector<voice> activeVoices = this->voiceManager->getActiveVoices();

    for (int i = 0; i < activeVoices.size(); i++) {
        voice& v = activeVoices[i];
        int id = v.getId();

        modelVoice& mV = this->allVoices[id - 1];
        signal.on ? mV.noteOn(signal.note) : mV.noteOff(signal.note);
    }
}

void modelModule::processSample(float& outL, float& outR) {
    float sum = 0.0f;

    for (auto& v : allVoices) {
        sum += v.process();
    }

    outL += sum;
    outR += sum;
}

void modelModule::load() {
    for (auto& v : this->voiceManager->getVoices()) {
        modelVoiceParams& params = v.getModelParams();
        modelVoice mV;
        mV.load(params);
        this->allVoices.push_back(mV);
    }
}

void modelModule::unload() {
    this->allVoices.clear();
}



modelVoice::modelVoice() : params() {}

void modelVoice::load(modelVoiceParams& params) {
    this->params = params;
    this->pipes.clear();

    for (int note = 0; note < 127; note++) {
        modelPipe p;
        modelPipeParams pParams = this->pipeParams(note);
        p.load(pParams);
        this->pipes.push_back(p);
    }
}

void modelVoice::noteOn(int note) {
    this->pipes[note].noteOn();
}

void modelVoice::noteOff(int note) {
    this->pipes[note].noteOff();
}

float modelVoice::process() {
    float out = 0.0f;

    for (auto& pipe : this->pipes) {
        out += pipe.process();
    }

    return out;
}

modelPipeParams modelVoice::pipeParams(int note) {
    modelPipeParams p;
    float freq = this->params.baseFrequency * std::pow(2.0f, (note - 69) / 12.0f) * this->params.scale;

    // TODO: More params

    return p;
}


void modelPipe::load(modelPipeParams& params) {
	this->params = params;
    
}

modelPipe::modelPipe() : params() {
    this->playing = false;
}

void modelPipe::noteOn() {
    this->playing = true;
}

void modelPipe::noteOff() {
    this->playing = false;
}

float modelPipe::process() {
    if (!this->playing)
	    return 0.0f;

    float out = 0.5f;

    // TODO: Implementacja

    return out;
}