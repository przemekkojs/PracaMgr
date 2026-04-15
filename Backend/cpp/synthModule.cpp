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

    outL += sum;
    outR += sum;
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

synthPipe::synthPipe() : params(), adsr() { }

void synthPipe::load(synthPipeParams& params) {
    this->params = params;

    this->adsr.setAttack(0.04f);
    this->adsr.setDecay(0.05f);
    this->adsr.setSustain(0.7f);
    this->adsr.setRelease(0.1f);

    delayLine.assign(static_cast<std::vector<float, std::allocator<float>>::size_type>((int)params.delaySamples) + 4, 0.0f);
    jetDelayLine.assign(static_cast<std::vector<float, std::allocator<float>>::size_type>((int)params.jetDelaySamples) + 4, 0.0f);

    writeIdx = 0;
    jetIdx = 0;
}

float synthPipe::pinkNoise() {
    static float b0 = 0, b1 = 0, b2 = 0;

    float white = noise(rng);

    b0 = 0.99886f * b0 + white * 0.0555179f;
    b1 = 0.99332f * b1 + white * 0.0750759f;
    b2 = 0.96900f * b2 + white * 0.1538520f;

    return (b0 + b1 + b2) * 0.33f;
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
    this->adsr.noteOn();

    for (int i = 0; i < delayLine.size(); i++) {
        delayLine[i] = 0.01f * ((float)rand() / RAND_MAX - 0.5f);
    }

    lossState = 0;
}

void synthPipe::noteOff() {
    this->adsr.noteOff();
}

float synthPipe::lossFilter(float x) {
    lossState = (1 - params.baseParams.lossFilterCoeff) * x + params.baseParams.lossFilterCoeff * lossState;
    return lossState;
}

float synthPipe::lowpass(float x) {
    loopLP = (1 - params.baseParams.lowpassCoeff) * x + params.baseParams.lowpassCoeff * loopLP;
    return loopLP;
}

float synthPipe::jetLowpass(float x) {
    jetLP = (1 - params.baseParams.jetLowpassCoeff) * x + params.baseParams.jetLowpassCoeff * jetLP;
    return jetLP;
}

float synthPipe::nonlinear(float x, float env) {
    float offset = 0.05f * (1 + env);
    float input = std::clamp(x + offset, -1.0f, 1.0f);
    return input - (input * input * input);
}

float synthPipe::process() {
    if (!adsr.isActive() || delayLine.empty() || jetDelayLine.empty()) {
        return 0.0f;
    }        

    float env = adsr.process();
    float noiseAmount = params.baseParams.noiseGain * env;

    if (env > 0.9f)
        noiseAmount *= 1.5f;

    float tap = (float)writeIdx - params.delaySamples;

    if (tap < 0)
        tap += (float)delayLine.size();

    int i1 = (int)tap;
    int i2 = (i1 + 1) % delayLine.size();
    float f = tap - (float)i1;
    float pipeOut = delayLine[i1] + f * (delayLine[i2] - delayLine[i1]);

    float filteredOut = pipeOut - lastSample;
    lastSample = pipeOut * 0.995f;   

    float turbulence = whiteNoise() * noiseAmount;    
    float driftNoise = ((float)rand() / RAND_MAX - 0.5f) * 0.002f;
    smoothedNoise = (0.05f * turbulence) + (0.95f * smoothedNoise);    
    windDrift = (0.9999f * windDrift) + driftNoise;
    float breath = (params.baseParams.excitationGain * env) + smoothedNoise + windDrift;
    float pressureDiff = breath - (pipeOut * params.baseParams.reflection);

    jetDelayLine[jetIdx] = pressureDiff;
    int jetReadIdx = (jetIdx + 1) % jetDelayLine.size();
    float jetDelayed = jetDelayLine[jetReadIdx];
    jetIdx = jetReadIdx;

    float x = std::clamp(jetDelayed, -1.0f, 1.0f);
    float nonlinearOut = this->nonlinear(x, env);
    float pipeInput = this->lowpass(nonlinearOut + (pipeOut * params.baseParams.loopFeedbackGain));

    delayLine[writeIdx] = pipeInput;
    writeIdx = (writeIdx + 1) % delayLine.size();

    float dcBlocker = pipeOut - lastPipeOut;
    lastPipeOut = pipeOut * 0.995f;

    return dcBlocker * 0.3f;
}


synthVoice::synthVoice() : pipes(), params() {}

synthPipeParams synthVoice::pipeParams(int note) {
    synthPipeParams p;

    float freq = this->params.baseFrequency * std::pow(2.0f, (note - 69) / 12.0f) * this->params.scale;
    float filterDelayComp = 1.0f;
    float jetRatio = 0.5f;

    p.baseParams = this->params;
    p.frequency = freq;
    p.delaySamples = (this->params.sampleRate / freq) - filterDelayComp;
    p.jetDelaySamples = p.delaySamples * jetRatio;
    p.jetDelaySamples = std::clamp(p.jetDelaySamples, 2.0f, p.delaySamples * 0.9f);

    return p;
}

void synthVoice::load(synthVoiceParams& params) {
    this->pipes.clear();
    this->params = params;

    for (int note = 0; note < 127; note++) {
        synthPipe p;
        synthPipeParams pParams = this->pipeParams(note);
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


float ADSR::process() {
    switch (state) {
        case AdsrState::ATTACK:
            value += attackRate;

            if (value >= 1.0f) { 
                value = 1.0f;
                state = AdsrState::DECAY;
            }

            break;
        case AdsrState::DECAY:
            value -= decayRate;

            if (value <= sustainLevel) {
                value = sustainLevel;
                state = AdsrState::SUSTAIN;
            }

            break;
        case AdsrState::SUSTAIN:
            break;
        case AdsrState::RELEASE:
            value -= releaseRate;

            if (value <= 0.0f) {
                value = 0.0f;
                state = AdsrState::IDLE;
            }

            break;
        default: break;
    }

    return value;
}

void ADSR::calculateRate(float& what, float seconds) const {
    what = (1.0f / (seconds * this->sampleRate));
}