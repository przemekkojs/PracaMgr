#include "../h/samplesModule.h"

samplesModule::samplesModule(std::shared_ptr<voices> voiceManager, int maxPolyphony) : module(std::move(voiceManager)), running(true)
{
    std::cout << "Samples module init" << std::endl;

    this->maxPolyphony = maxPolyphony;
    this->initEngine();
    this->initDevice();
    this->voiceThread = std::thread([this]() { this->voiceManagerThread(); });
}

samplesModule::~samplesModule()
{
    std::cout << "Destructor of Samples module" << std::endl;

    running = false;
    if (voiceThread.joinable())
        voiceThread.join();

    for (auto& [key, s] : samples) {
        delete s;
    }

    ma_device_uninit(&device);
    ma_engine_uninit(&engine);

    std::cout << "Destructed" << std::endl;
}

void samplesModule::initDevice() {
    std::cout << "Device init" << std::endl;
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate = 44100;
    deviceConfig.dataCallback = audioCallback;
    deviceConfig.pUserData = this;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        throw std::runtime_error("Failed to init device");
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        throw std::runtime_error("Failed to start device");
    }
}

void samplesModule::initEngine()
{
    std::cout << "Samples engine init" << std::endl;

    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        throw std::runtime_error("Engine init failed!");
    }

    int loadedSamples = 0;
    const int predictedSamplesCount = this->voiceManager->getVoices().size() * NUMBER_OF_NOTES;

    for (const auto& v : this->voiceManager->getVoices()) {
        for (int note = LOWEST_NOTE; note < (LOWEST_NOTE + NUMBER_OF_NOTES); note++) {
            std::vector<std::string> paths = v.getSamplesPath(note);

            if (paths.size() != 3) {
                std::cout << "Unable to load samples for note " << note << std::endl;
                continue;
            }

            std::string sustainPath = paths[1];

            sample* s = new sample();
            ma_decoder decoder;
            ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);

            if (ma_decoder_init_file(sustainPath.c_str(), &config, &decoder) != MA_SUCCESS) {
                std::cout << "Failed to open file: " << sustainPath << std::endl;
                delete s;
                continue;
            }

            ma_uint64 frameCount;
            ma_decoder_get_length_in_pcm_frames(&decoder, &frameCount);

            s->data.resize(frameCount * decoder.outputChannels);

            ma_uint64 framesRead;
            if (ma_decoder_read_pcm_frames(&decoder, s->data.data(), frameCount, &framesRead) != MA_SUCCESS) {
                std::cout << "Failed to read file: " << sustainPath << std::endl;
                ma_decoder_uninit(&decoder);
                delete s;
                continue;
            }

            ma_decoder_uninit(&decoder);

            s->frameCount = framesRead;
            s->channels = decoder.outputChannels;
            s->sampleRate = decoder.outputSampleRate;
            s->loaded = true;
            s->note = note;
            s->voiceId = v.getId();

            samples[{v.getId(), note}] = s;
            loadedSamples++;
        }
    }

    ma_engine_set_volume(&engine, 1.0f);
    std::cout << "Successfully loaded " << loadedSamples << " of " << predictedSamplesCount << " samples" << std::endl;
}

void samplesModule::play(const noteSignal& signal, audioSignal&)
{
    int note = signal.note;

    if (signal.on) {
        for (const auto& v : this->voiceManager->getActiveVoices()) {
            int id = v.getId();
            auto it = samples.find({ id, note });
            if (it == samples.end()) continue;

            sample* s = it->second;
            if (!s || !s->loaded) continue;            

            sampleVoice voice;
            voice.s = s;
            voice.note = note;
            voice.cursor = 0.0f;
            voice.increment = 1.0f;
            voice.loopStart = s->sampleRate;
            voice.loopEnd = s->frameCount - s->sampleRate;
            voice.fadeLength = s->sampleRate;
            voice.looping = true;
            voice.active = true;

            std::lock_guard<std::mutex> lock(queueMutex);
            newVoicesQueue.push_back(voice);
        }
    }
    else
    {
        std::lock_guard<std::mutex> lock(voicesMutex);

        for (auto& v : activeVoices) {
            if (!v.active ||
                v.s == nullptr)
                continue;

            auto it = this->samples.find({ v.s->voiceId, v.s->note });
            v.active = false;
        }
    }
}

void samplesModule::getSample(sampleVoice& v, float& outL, float& outR)
{
    outL = 0.0f;
    outR = 0.0f;

    if (!v.active || !v.s || !v.s->loaded || v.s->frameCount < 2)
        return;

    auto& data = v.s->data;
    uint32_t ch = v.s->channels;
    uint64_t i1 = (uint64_t)v.cursor;
    float frac = v.cursor - (float)i1;

    auto sampleAt = [&](uint64_t idx, float& l, float& r) {
        uint64_t iA = idx;
        uint64_t iB = (idx + 1 < v.s->frameCount) ? idx + 1 : idx;
        uint64_t baseA = iA * ch;
        uint64_t baseB = iB * ch;

        float f = frac;       

        if (ch == 1) {
            float s1 = data[baseA];
            float s2 = data[baseB];
            float val = s1 * (1.0f - f) + s2 * f;
            l = val;
            r = val;
        }
        else {
            float l1 = data[baseA + 0];
            float r1 = data[baseA + 1];
            float l2 = data[baseB + 0];
            float r2 = data[baseB + 1];

            l = l1 * (1.0f - f) + l2 * f;
            r = r1 * (1.0f - f) + r2 * f;
        }
    };

    if (!v.looping || v.cursor < v.loopEnd - v.fadeLength) {
        sampleAt(i1, outL, outR);
    }
    else {
        float fadePos = (v.cursor - (v.loopEnd - v.fadeLength)) / (float)v.fadeLength;
        float l1, r1;
        float l2, r2;       

        uint64_t loopPos = (i1 - (v.loopEnd - v.fadeLength));

        if (loopPos >= v.loopEnd - 1)
            loopPos = v.loopStart;
        
        sampleAt(i1, l1, r1);
        sampleAt(loopPos, l2, r2);

        float w1 = 1.0f - fadePos;
        float w2 = fadePos;

        outL = l1 * w1 + l2 * w2;
        outR = r1 * w1 + r2 * w2;
    }

    v.cursor += v.increment;

    if (v.looping && v.cursor >= v.loopEnd) {
        v.cursor = v.loopStart + (v.cursor - v.loopEnd);
    }

    if (!v.looping && v.cursor >= v.s->frameCount) {
        v.active = false;
    }
}

void samplesModule::audioCallback(ma_device* pDevice, void* pOutput, const void*, ma_uint32 frameCount)
{
    auto* module = (samplesModule*)pDevice->pUserData;
    float* out = (float*)pOutput;

    std::lock_guard<std::mutex> lock(module->voicesMutex);
    auto& voices = module->getActiveVoices();

    for (ma_uint32 i = 0; i < frameCount; i++)
    {
        float outL = 0.0f;
        float outR = 0.0f;

        for (auto& v : voices)
        {
            if (!v.active) continue;
            float l, r;
            module->getSample(v, l, r);
            outL += l;
            outR += r;
        }

        out[i * 2 + 0] = outL * 0.2f;
        out[i * 2 + 1] = outR * 0.2f;
    }
}

void samplesModule::voiceManagerThread()
{
    while (running)
    {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            if (!newVoicesQueue.empty())
            {
                std::lock_guard<std::mutex> lock2(voicesMutex);
                for (auto& v : newVoicesQueue)
                    activeVoices.push_back(v);

                newVoicesQueue.clear();
            }

            std::lock_guard<std::mutex> lock2(voicesMutex);
            activeVoices.erase(
                std::remove_if(activeVoices.begin(), activeVoices.end(),
                    [](const sampleVoice& v) { return !v.active; }),
                activeVoices.end()
            );
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}