#include "../h/synthModule.h"

#include <iostream>

synthModule::synthModule(std::shared_ptr<voices> voiceManager) : module(std::move(voiceManager)), allVoices() {

}

void synthModule::play(const noteSignal& signal) {
    if (signal.on) {
        for (auto& v : this->allVoices) {
            auto& voice = v.second;

            if (!voice.playing) {
                voice.noteOn(signal.note);
            }
        }
    }
    else {
        for (auto& v : allVoices) {
            auto& voice = v.second;

            if (voice.playing) {
                voice.noteOff();
            }
        }
    }
}

void synthModule::processSample(float& outL, float& outR) {
    float mix = 0.0f;

    for (auto& v : this->voiceManager->getActiveVoices()) {
        if (!v.isActive())
            continue;

        int id = v.getId();
        auto it = allVoices.find(id);

        if (it != allVoices.end()) {
            mix += it->second.process();
        }
    }

    outL = mix;
    outR = mix;
}

void synthModule::load() {
    for (auto& voice : this->voiceManager->getVoices()) {
        pipeParams& params = voice.getSynthParams();
        this->allVoices.emplace(voice.getId(), synthVoice(params));

        std::cout << "Loaded " << voice.getName() << std::endl;
    }
}

void synthModule::unload() {
    this->allVoices.clear();
}


static inline float midiToFreq(int midi) {
    return 440.0f * std::pow(2.0f, (midi - 69) / 12.0f);
}

synthVoice::synthVoice(const pipeParams& p) : params(p) {
    this->sampleRate = module::SAMPLE_RATE;
    this->env = 0.0f;
    this->feedback = 0.0f;
    this->phaseReset = true;    
}

void synthVoice::noteOn(int midiNote)
{
    freq = midiToFreq(midiNote);

    playing = true;

    env = 0.0f;
    feedback = 0.0f;

    pipeDelay.reset(0.02f);
    jetDelay.reset(0.01f);
    pipeDelay.resize(48000);
    jetDelay.resize(2048);

    lossFilter.setLowpass(params.loss);
}

float synthVoice::process()
{
    if (!playing)
        return 0.0f;

    env += (1.0f - env) * params.envSpeed;
    float periodSamples = sampleRate / freq;
    float detune = 1.0f + params.detune * sinf(0.0001f * feedback);

    float delaySamples = periodSamples * params.pipeScale * params.tuning * detune;
    float excitation = (noise() * params.noiseWeight + sinf(feedback * 5.0f) * (1.0f - params.noiseWeight)) * env * params.noiseGain;
    float input = excitation + feedback * (params.feedbackGain);
    float nl = tanh(input);
    float jetOut = jetDelay.read(params.jetDelay);
    jetDelay.write(nl);
    float pipeOut = pipeDelay.read(delaySamples);
    pipeDelay.write(jetOut);
    float filtered = lossFilter.process(pipeOut);
    feedback = filtered * params.feedbackDamping;
    float out = filtered * params.outputGain;

    return out;
}