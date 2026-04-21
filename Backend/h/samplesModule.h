#pragma once

#include "module.h"
#include "config.h"
#include "../lib/miniaudio.h"

#include <map>
#include <queue>
#include <algorithm>
#include <mutex>
#include <thread>
#include <chrono>
#include <memory>

struct sample {
    int voiceId = 0;
    int note = 0;

    std::vector<float> data;
    uint64_t frameCount = 0;
    uint32_t channels = 0;
    uint32_t sampleRate = SAMPLE_RATE;
    bool loaded = false;
};

struct sampleVoice {
    sample* s = nullptr;
    int note = -1;
    float cursor = 0.0f;
    float increment = 1.0f;
    uint64_t loopStart = 0;
    uint64_t loopEnd = 0;
    uint32_t fadeLength = 0;
    bool looping = false;
    bool active = false;
};

struct stereoSample {
    float l = 0.0f;
    float r = 0.0f;
};

class samplesModule : public module {
public:
    samplesModule(std::shared_ptr<voices> voiceManager);
    ~samplesModule();

    void play(const noteSignal& signal) override;
    void processSample(float& outL, float& outR) override;
    void load() override;
    void unload() override;    

    const std::map<std::pair<int, int>, sample*>& getSamples() const { return samples; }
    std::vector<sampleVoice>& getActiveVoices() { return activeVoices; }
    std::mutex& getVoicesMutex() { return voicesMutex; }
private:
    std::map<std::pair<int, int>, sample*> samples;
    std::vector<sampleVoice> newVoicesQueue;
    std::vector<sampleVoice> activeVoices;
    std::mutex queueMutex;    
    std::mutex voicesMutex;
    std::thread voiceThread;
    
    bool running;

    void voiceManagerThread();
    void getSample(sampleVoice& v, float& outL, float& outR);    
    void loadSamples();
    void unloadSamples();
};