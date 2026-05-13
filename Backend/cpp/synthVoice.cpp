#include "../h/synthVoice.h"

synthVoice::synthVoice() : pipes(), params() {}

synthPipeParams synthVoice::pipeParams(int note) const {
    synthPipeParams p;

    float freq = this->params.baseFrequency * std::pow(2.0f, (note - 69) / 12.0f) * this->params.scale;
    float filterDelayComp = 1.0f;

    p.baseParams = this->params;
    p.frequency = freq;
    p.delaySamples = (SAMPLE_RATE / freq) - filterDelayComp;
    p.jetDelaySamples = p.delaySamples * this->params.jetLength;
    p.jetDelaySamples = std::clamp(p.jetDelaySamples, 2.0f, p.delaySamples * 0.9f);

    return p;
}

void synthVoice::load(const synthVoiceParams& params, voiceType vT, bool isModel) {
    this->pipes.clear();
    this->params = params;

    for (int note = 0; note < 127; note++) {
        switch (vT) {
            case FLUTE: {
                if (isModel) {
                    std::unique_ptr<flutePipeModel> p = std::make_unique<flutePipeModel>();
                    synthPipeParams pParams = this->pipeParams(note);
                    p->load(pParams);
                    this->pipes.push_back(std::move(p));
                }
                else {
                    std::unique_ptr<flutePipe> p = std::make_unique<flutePipe>();
                    synthPipeParams pParams = this->pipeParams(note);
                    p->load(pParams);
                    this->pipes.push_back(std::move(p));
                }
                
                
                break;
            }

            case STRING: {
                if (isModel) {
                    std::unique_ptr<stringPipeModel> p = std::make_unique<stringPipeModel>();
                    synthPipeParams pParams = this->pipeParams(note);
                    p->load(pParams);
                    this->pipes.push_back(std::move(p));
                }
                else {
                    std::unique_ptr<stringPipe> p = std::make_unique<stringPipe>();
                    synthPipeParams pParams = this->pipeParams(note);
                    p->load(pParams);
                    this->pipes.push_back(std::move(p));
                }
                
                break;
            }

            case PRINCIPAL: {
                if (isModel) {
                    std::unique_ptr<principalPipeModel> p = std::make_unique<principalPipeModel>();
                    synthPipeParams pParams = this->pipeParams(note);
                    p->load(pParams);
                    this->pipes.push_back(std::move(p));
                }
                else {
                    std::unique_ptr<principalPipe> p = std::make_unique<principalPipe>();
                    synthPipeParams pParams = this->pipeParams(note);
                    p->load(pParams);
                    this->pipes.push_back(std::move(p));
                }
                
                break;
            }

            case REED: {
                if (isModel) {
                    std::unique_ptr<reedPipeModel> p = std::make_unique<reedPipeModel>();
                    synthPipeParams pParams = this->pipeParams(note);
                    p->load(pParams);
                    this->pipes.push_back(std::move(p));
                }
                else {
                    std::unique_ptr<reedPipe> p = std::make_unique<reedPipe>();
                    synthPipeParams pParams = this->pipeParams(note);
                    p->load(pParams);
                    this->pipes.push_back(std::move(p));
                }
                
                break;
            }
        }        
    }
}

void synthVoice::noteOn(int note) {
    this->pipes[note]->noteOn();
}

void synthVoice::noteOff(int note) {
    this->pipes[note]->noteOff();
}

float synthVoice::process() {
    float out = 0.0f;

    for (auto& pipe : this->pipes) {
        out += pipe->process();
    }

    return out;
}