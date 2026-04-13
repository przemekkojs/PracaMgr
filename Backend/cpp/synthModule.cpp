#include "../h/synthModule.h"

#include <iostream>

synthModule::synthModule(std::shared_ptr<voices> voiceManager) : module(std::move(voiceManager)), allVoices() {

}

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

    outL += sum * 0.2f;
    outR += sum * 0.2f;
}

void synthModule::load() {
    for (auto& v : this->voiceManager->getVoices()) {
        synthVoiceParams& params = v.getSynthParams();
        synthVoice sV;
        sV.load(params);
        this->allVoices.push_back(sV);

        std::cout << "Loaded " << v.getName() << std::endl;
    }
}

void synthModule::unload() {
    this->allVoices.clear();
}


synthPipe::synthPipe() : params() {
    this->playing = false;
}

void synthPipe::load(synthPipeParams& params) {
    this->params = params;

    delayLine.assign(params.delaySamples, 0.0f);
    jetDelayLine.assign(params.jetDelaySamples, 0.0f);

    writeIdx = 0;
    jetIdx = 0;
}

float synthPipe::pinkNoise() {
    static float b0 = 0, b1 = 0, b2 = 0;

    float white = noise(rng);

    b0 = 0.99886f * b0 + white * 0.0555179f;
    b1 = 0.99332f * b1 + white * 0.0750759f;
    b2 = 0.96900f * b2 + white * 0.1538520f;

    return (b0 + b1 + b2) * 0.3f;
}

float synthPipe::brownNoise() {
    static float y = 0.0f;
    float white = noise(rng);

    y += white * 0.02f;
    y = std::clamp(y, -1.0f, 1.0f);

    return y * 3.5f;
}

float synthPipe::whiteNoise() {
    return noise(rng);
}

void synthPipe::noteOn() {
    this->playing = true;

    for (int i = 0; i < delayLine.size(); i++) {
        delayLine[i] = 0.01f * ((float)rand() / RAND_MAX - 0.5f);
    }

    lossState = 0;
}

void synthPipe::noteOff() {
    this->playing = false;
}

float synthPipe::lossFilter(float x) {
    lossState = (1 - params.loss) * x + params.loss * lossState;
    return lossState;
}

float synthPipe::lowpass(float x) {
    const float a = 0.12f;
    loopLP = (1 - a) * x + a * loopLP;
    return loopLP;
}

float synthPipe::jetLowpass(float x) {
    const float a = 0.25f;
    jetLP = (1 - a) * x + a * jetLP;
    return jetLP;
}

float synthPipe::nonlinear(float x) {
    return x - 0.9f * x * x * x;
}

float synthPipe::process() {
    if (!playing)
        return 0.0f;

    int readIdx = (writeIdx + 1) % delayLine.size();
    float pipeOut = delayLine[readIdx];
    float filtered = lossFilter(pipeOut);
    lastSample = filtered;

    float noiseIn = whiteNoise();
    noiseIn = jetLowpass(noiseIn);

    int jetRead = (jetIdx + 1) % jetDelayLine.size();
    float jetInput = params.noiseGain * noiseIn + params.jetGain * filtered;
    float jetDelayed = jetDelayLine[jetRead];
    jetDelayLine[jetIdx] = jetInput;
    jetIdx = jetRead;

    jetDelayed = jetLowpass(jetDelayed);
    float nonlinear = this->nonlinear(jetDelayed);
    float feedback = filtered * params.reflection;
    feedback = lowpass(feedback);

    float input = nonlinear + feedback;
    input = lowpass(input);
    input = lossFilter(input);

    delayLine[writeIdx] = input;
    writeIdx = readIdx;

    return pipeOut;
}

synthVoice::synthVoice() : pipes(), params() {}

synthPipeParams& synthVoice::pipeParams(int note) {
    static synthPipeParams p;

    float freq = this->params.baseFrequency *
        std::pow(2.0f, (note - 69) / 12.0f);

    freq *= this->params.scale;

    p.frequency = freq;
    p.sampleRate = this->params.sampleRate;
    p.reflection = this->params.reflection;
    p.loss = this->params.loss;
    p.excitationGain = this->params.excitationGain;
    p.noiseGain = this->params.noiseGain;
    p.jetGain = this->params.jetGain;
    p.delaySamples = (int)(p.sampleRate / p.frequency);
    p.jetDelaySamples = (int)(this->params.jetLength * p.delaySamples);

    return p;
}

void synthVoice::load(synthVoiceParams& params) {
    this->pipes.clear();
    this->params = params;

    for (int note = 0; note < 127; note++) {
        synthPipe p;
        synthPipeParams pParams = pipeParams(note);
        p.load(pParams);
        this->pipes.push_back(p);
    }
}

void synthVoice::noteOn(int note) {
    this->pipes[note].noteOn();
}

void synthVoice::noteOff(int note) {
    this->pipes[note].noteOff();
}

float synthVoice::process() {
    float out = 0.0f;

    for (auto& pipe : this->pipes) {
        out += pipe.process();
    }

    return out;
}