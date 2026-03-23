#include "../h/samplesModule.h"
#include <iostream>
#include <cstring>

samplesModule::samplesModule(std::shared_ptr<voices> voiceManager, int maxPolyphony)
    : module(std::move(voiceManager)), maxPolyphony(maxPolyphony)
{
    std::cout << "Samples module init\n";

    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_f32;
    config.playback.channels = 2;
    config.sampleRate = 48000;
    config.dataCallback = audioCallback;
    config.pUserData = this;

    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
        throw std::runtime_error("Device init failed");
    }

    activeVoices.resize(maxPolyphony);

    initEngine();

    ma_device_start(&device);
}

samplesModule::~samplesModule()
{
    std::cout << "Destructor of Samples module\n";

    ma_device_uninit(&device);

    for (auto& [key, s] : samples) {
        free(s.s1.data);
        free(s.s2.data);
    }

    std::cout << "Destructed\n";
}

void samplesModule::initEngine()
{
    std::cout << "Loading samples...\n";

    for (const auto& v : this->voiceManager->getVoices()) {
        for (int note = LOWEST_NOTE; note < LOWEST_NOTE + NUMBER_OF_NOTES; note++) {

            auto paths = v.getSamplesPath(note);
            if (paths.size() < 2) continue;

            sampleSet s{};
            ma_uint32 ch1, ch2;

            if (!loadSampleToBuffer(paths[1].c_str(), &s.s1, &ch1)) continue;
            if (!loadSampleToBuffer(paths[1].c_str(), &s.s2, &ch2)) continue;

            if (ch1 != ch2) continue;

            s.channels = ch1;
            s.crossfadeFrames = 1024;

            samples[{v.getId(), note}] = s;
        }
    }

    std::cout << "Loaded samples: " << samples.size() << "\n";
}

void samplesModule::play(const noteSignal& signal, audioSignal&)
{
    int note = signal.note;

    for (const auto& v : this->voiceManager->getActiveVoices()) {
        int id = v.getId();

        auto it = samples.find({ id, note });
        if (it == samples.end()) continue;

        for (auto& av : activeVoices) {
            if (!av.active) {
                av.sample = it->second;

                av.sample.cursor1 = 0;
                av.sample.cursor2 = 0;
                av.sample.useFirst = true;

                av.active = true;
                return;
            }
        }
    }
}

void samplesModule::audioCallback(ma_device* device, void* output, const void*, ma_uint32 frameCount)
{
    auto* self = (samplesModule*)device->pUserData;
    float* out = (float*)output;

    const ma_uint32 channels = 2;

    std::memset(out, 0, frameCount * channels * sizeof(float));

    for (auto& v : self->activeVoices) {
        if (!v.active) continue;

        processSampleSet(&v.sample, out, frameCount);
    }
}

void samplesModule::processSampleSet(sampleSet* s, float* out, ma_uint32 frameCount)
{
    SampleBuffer* cur = s->useFirst ? &s->s1 : &s->s2;
    SampleBuffer* next = s->useFirst ? &s->s2 : &s->s1;

    ma_uint64* ccur = s->useFirst ? &s->cursor1 : &s->cursor2;
    ma_uint64* cnext = s->useFirst ? &s->cursor2 : &s->cursor1;
    ma_uint64 posCur = (*ccur < cur->length) ? *ccur : (cur->length - 1);
    ma_uint64 posNext = (*cnext < next->length) ? *cnext : (next->length - 1);

    const ma_uint32 ch = s->channels;

    for (ma_uint32 i = 0; i < frameCount; i++) {

        ma_uint64 remaining = cur->length - *ccur;

        float t = 0.0f;
        if (remaining <= s->crossfadeFrames) {
            t = 1.0f - (float)remaining / (float)s->crossfadeFrames;
        }

        for (ma_uint32 c = 0; c < ch; c++) {

            float a = cur->data[posCur * ch + c];
            float b = next->data[posNext * ch + c];

            float gainA = cosf(t * 1.5707963f);
            float gainB = sinf(t * 1.5707963f);

            out[i * ch + c] += a * gainA + b * gainB;
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

bool samplesModule::loadSampleToBuffer(const char* file, SampleBuffer* out, ma_uint32* channels)
{
    ma_decoder decoder;
    ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 2, 48000);

    if (ma_decoder_init_file(file, &config, &decoder) != MA_SUCCESS)
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