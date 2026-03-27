#pragma once

#include "module.h"
#include "../lib/miniaudio.h"

#include <map>
#include <queue>
#include <iostream>
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
    uint32_t sampleRate = 44100;
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
    samplesModule(std::shared_ptr<voices> voiceManager, int maxPolyphony = 1024);
    ~samplesModule();

    void play(const noteSignal& signal, audioSignal& output) override;
    void load() override;
    void unload() override;
    void loadSamples();
    void initEngine();
    void initDevice();    

    const std::map<std::pair<int, int>, sample*>& getSamples() const { return samples; }
    std::vector<sampleVoice>& getActiveVoices() { return activeVoices; }

    std::mutex& getVoicesMutex() { return voicesMutex; }

private:
    std::map<std::pair<int, int>, sample*> samples;

    ma_engine engine;
    ma_device device;

    int maxPolyphony;
    bool moduleActive;

    std::vector<sampleVoice> newVoicesQueue;
    std::vector<sampleVoice> activeVoices;
    std::mutex queueMutex;    
    std::mutex voicesMutex;
    std::thread voiceThread;
    void voiceManagerThread();
    bool running;    

    void getSample(sampleVoice& v, float& outL, float& outR);
    static void audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
};