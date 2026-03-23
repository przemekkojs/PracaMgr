#include "../h/samplesModule.h"
#include <iostream>
#include <algorithm>


samplesModule::samplesModule(std::shared_ptr<voices> voiceManager, int maxPolyphony) : module(std::move(voiceManager)) {
    std::cout << "Samples module init" << std::endl;
    this->maxPolyphony = maxPolyphony;

    if (ma_engine_init(NULL, &this->engine) != MA_SUCCESS) {
        throw std::runtime_error("Engine init failed!");
    }

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_f32;
    config.playback.channels = 2;
    config.sampleRate = 48000;
    config.dataCallback = audioCallback;
    config.pUserData = this;

    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
        throw std::runtime_error("Device init failed");
    }       

    ma_device_start(&device);

    activeVoices.resize(maxPolyphony);

    for (auto& v : activeVoices)
        v.active = false;

    this->initEngine();
}

void samplesModule::initEngine() {
    std::cout << "Samples engine init" << std::endl;

    for (const auto& v : this->voiceManager->getVoices()) {
        for (int note = LOWEST_NOTE; note < (LOWEST_NOTE + NUMBER_OF_NOTES); note++) {

            std::vector<std::string> paths = v.getSamplesPath(note);

            sampleSet s{};
            ma_uint32 ch1, ch2;

            if (!loadSampleToBuffer(&engine, paths[1].c_str(), &s.s1, &ch1)) continue;
            if (!loadSampleToBuffer(&engine, paths[1].c_str(), &s.s2, &ch2)) continue;

            if (ch1 != ch2) continue;

            s.channels = ch1;
            s.cursor1 = 0;
            s.cursor2 = 0;
            s.useFirst = true;
            s.crossfadeFrames = 1024;

            samples[{v.getId(), note}] = s;
        }
    }

    std::cout << "Loaded samples: " << samples.size() << std::endl;
}

samplesModule::~samplesModule() {
    std::cout << "Destructor of Samples module" << std::endl;

    ma_device_uninit(&device);
    ma_engine_uninit(&engine);

    std::cout << "Destructed" << std::endl;
}

void samplesModule::play(const noteSignal& signal, audioSignal&) {
    int note = signal.note;

    for (const auto& v : this->voiceManager->getActiveVoices()) {
        int id = v.getId();

        auto it = samples.find({ id, note });
        if (it == samples.end()) continue;

        for (auto& av : activeVoices) {
            if (!av.active) {
                av.sample = &it->second;
                av.sample->cursor1 = 0;
                av.sample->cursor2 = 0;
                av.sample->useFirst = true;

                av.active = true;
                return;
            }
        }
    }
}



void audioCallback(ma_device* device, void* output, const void*, ma_uint32 frameCount)
{
    sampleSet* s = (sampleSet*)device->pUserData;
    float* out = (float*)output;

    memset(out, 0, frameCount * s->channels * sizeof(float));

    processSampleSet(s, out, frameCount);
}

bool loadSampleToBuffer(ma_engine* engine, const char* file, SampleBuffer* out, ma_uint32* channels)
{
    ma_decoder decoder;
    if (ma_decoder_init_file(file, NULL, &decoder) != MA_SUCCESS)
        return false;

    *channels = decoder.outputChannels;

    ma_uint64 totalFrames;
    ma_decoder_get_length_in_pcm_frames(&decoder, &totalFrames);

    float* data = (float*)malloc(sizeof(float) * totalFrames * (*channels));

    if (!data) {
        ma_decoder_uninit(&decoder);
        return false;
    }

    ma_uint64 framesRead;
    ma_decoder_read_pcm_frames(&decoder, data, totalFrames, &framesRead);

    ma_decoder_uninit(&decoder);

    out->data = data;
    out->length = framesRead;

    return true;
}

bool initSampleSet(ma_engine* engine, sampleSet* s, const char* f1, const char* f2)
{
    ma_uint32 ch1, ch2;

    if (!loadSampleToBuffer(engine, f1, &s->s1, &ch1)) return false;
    if (!loadSampleToBuffer(engine, f2, &s->s2, &ch2)) return false;

    if (ch1 != ch2) return false;

    s->channels = ch1;

    s->cursor1 = 0;
    s->cursor2 = 0;
    s->useFirst = true;

    s->crossfadeFrames = 1024;

    return true;
}

inline void processSampleSet(sampleSet* s, float* out, ma_uint32 frameCount)
{
    SampleBuffer* cur = s->useFirst ? &s->s1 : &s->s2;
    SampleBuffer* next = s->useFirst ? &s->s2 : &s->s1;

    ma_uint64* ccur = s->useFirst ? &s->cursor1 : &s->cursor2;
    ma_uint64* cnext = s->useFirst ? &s->cursor2 : &s->cursor1;

    const ma_uint32 ch = s->channels;

    for (ma_uint32 i = 0; i < frameCount; i++) {

        ma_uint64 posCur = *ccur;
        ma_uint64 posNext = *cnext;

        ma_uint64 remaining = cur->length - posCur;

        float t = 0.0f;

        if (remaining <= s->crossfadeFrames) {
            t = 1.0f - (float)remaining / (float)s->crossfadeFrames;
        }

        for (ma_uint32 c = 0; c < ch; c++) {

            float a = cur->data[posCur * ch + c];
            float b = next->data[posNext * ch + c];

            out[i * ch + c] += a * (1.0f - t) + b * t;
        }

        (*ccur)++;
        if (remaining <= s->crossfadeFrames)
            (*cnext)++;

        if (*ccur >= cur->length) {
            *ccur = 0;
            s->useFirst = !s->useFirst;
        }

        if (*cnext >= next->length) {
            *cnext = 0;
        }
    }
}