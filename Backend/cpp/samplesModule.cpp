#include "../h/samplesModule.h"
#include <iostream>
#include <algorithm>

ma_result LoopingSample_getDataFormat(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel*, size_t) {
    LoopingSample* s = (LoopingSample*)pDataSource;

    *pFormat = ma_format_f32;
    *pChannels = s->channels;
    *pSampleRate = s->sampleRate;
    return MA_SUCCESS;
}

ma_result LoopingSample_seek_to_pcm_frame(ma_data_source* pDataSource, ma_uint64 frameIndex) {
    LoopingSample* s = (LoopingSample*)pDataSource;
    if (frameIndex >= s->frameCount) return MA_ERROR;
    s->cursor = frameIndex;
    return MA_SUCCESS;
}

ma_result LoopingSample_read_pcm_frames(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead) {
    LoopingSample* s = (LoopingSample*)pDataSource;
    float* out = (float*)pFramesOut;
    ma_uint64 framesAvailable = s->frameCount - s->cursor;
    ma_uint64 framesToCopy = std::min(frameCount, framesAvailable);

    std::copy(s->data + s->cursor * s->channels, s->data + (s->cursor + framesToCopy) * s->channels, out);
    s->cursor += framesToCopy;

    if (pFramesRead)
        *pFramesRead = framesToCopy;
    return MA_SUCCESS;
}

ma_data_source_vtable g_looping_vtable = {
    LoopingSample_read_pcm_frames,
    LoopingSample_seek_to_pcm_frame,
    LoopingSample_getDataFormat,
    nullptr,
    nullptr,
    nullptr,
    0
};

// prosta funkcja miksuj¹ca do stereo
static void audioCallback(ma_device* pDevice, void* pOutput, const void*, ma_uint32 frameCount) {
    samplesModule* module = (samplesModule*)pDevice->pUserData;
    float* out = (float*)pOutput;

    // wyzeruj bufor wyjœciowy
    std::fill(out, out + frameCount * 2, 0.0f); // stereo

    // dla wszystkich próbek w module
    for (auto& [key, set] : module->getSamples()) {
        set->updateLooping(); // aktualizacja pêtli i crossfade

        // prosty miks – minimalny przyk³ad, zak³ada, ¿e sustain1 i sustain2 s¹ w³¹czone
        // jeœli graj¹, pobieramy z nich próbki i dodajemy do out
        // tutaj zostawiamy puste, bo ma_sound odtwarza samodzielnie
        // jeœli chcesz miksowaæ rêcznie, musisz u¿yæ ma_sound_read_pcm_frames
    }
}

samplesModule::samplesModule(std::shared_ptr<voices> voiceManager, int maxPolyphony) : module(std::move(voiceManager)) {
    std::cout << "Samples module init" << std::endl;

    this->maxPolyphony = maxPolyphony;

    if (ma_engine_init(NULL, &this->engine) != MA_SUCCESS) {
        throw std::runtime_error("Engine init failed!");
    }

    this->initEngine();

    std::cout << "Device init" << std::endl;
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate = 44100;
    deviceConfig.dataCallback = audioCallback;
    deviceConfig.pUserData = this;

    if (ma_device_init(NULL, &deviceConfig, &this->device) != MA_SUCCESS) {
        throw std::runtime_error("Failed to init device");
    }

    if (ma_device_start(&this->device) != MA_SUCCESS) {
        throw std::runtime_error("Failed to start device");
    }

    std::cout << "Device initialised" << std::endl;
}

void samplesModule::initEngine() {
    std::cout << "Samples engine init" << std::endl;
    int loadedSamples = 0;
    const int predictedSamplesCount = this->voiceManager->getVoices().size() * NUMBER_OF_NOTES;

    for (const auto& v : this->voiceManager->getVoices()) {
        for (int note = LOWEST_NOTE; note < (LOWEST_NOTE + NUMBER_OF_NOTES); note++) {
            std::vector<std::string> paths = v.getSamplesPath(note);
            std::string attackPath = paths[0];
            std::string sustainPath = paths[1];
            std::string releasePath = paths[2];
            sampleSet* set = new sampleSet();

            if (ma_sound_init_from_file(&engine, attackPath.c_str(), 0, NULL, NULL, &set->attack) != MA_SUCCESS) {
                std::cout << "Failed to load attack: " << attackPath << std::endl;
                delete set;
                continue;
            }

            ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
            ma_decoder decoder;

            if (ma_decoder_init_file(sustainPath.c_str(), &config, &decoder) != MA_SUCCESS) {
                std::cout << "Decoder init failed" << std::endl;
                delete set;
                continue;
            }

            ma_uint64 frameCount;
            ma_decoder_get_length_in_pcm_frames(&decoder, &frameCount);

            if (frameCount == 0) {
                std::cout << "Sustain sample empty: " << sustainPath << std::endl;
                ma_decoder_uninit(&decoder);
                delete set;
                continue;
            }

            float* pcm = new float[frameCount * decoder.outputChannels];
            ma_decoder_read_pcm_frames(&decoder, pcm, frameCount, nullptr);

            set->looping.data = pcm;
            set->looping.frameCount = frameCount;
            set->looping.base.vtable = &g_looping_vtable;
            set->looping.channels = decoder.outputChannels;
            set->looping.sampleRate = decoder.outputSampleRate;

            set->looping.loopStart = 0;
            set->looping.loopEnd = frameCount;
            set->looping.cursor = set->looping.loopStart;
            set->looping.fadeLength = std::min<ma_uint64>(frameCount / 32, frameCount - 1);

            if (ma_sound_init_from_data_source(&engine, &set->looping, 0, NULL, &set->sustain1) != MA_SUCCESS ||
                ma_sound_init_from_data_source(&engine, &set->looping, 0, NULL, &set->sustain2) != MA_SUCCESS) {
                delete[] pcm;
                delete set;
                ma_decoder_uninit(&decoder);
                continue;
            }

            ma_sound_set_volume(&set->sustain1, 1.0f);
            ma_sound_set_volume(&set->sustain2, 0.0f);
            ma_decoder_uninit(&decoder);

            if (ma_sound_init_from_file(&engine, releasePath.c_str(), 0, NULL, NULL, &set->release) != MA_SUCCESS) {
                ma_sound_uninit(&set->attack);
                ma_sound_uninit(&set->sustain1);
                ma_sound_uninit(&set->sustain2);

                delete[] pcm;
                delete set;
                continue;
            }

            loadedSamples += 1;
            samples.emplace(std::pair{ v.getId(), note }, set);
        }
    }

    ma_engine_set_volume(&engine, 1.0f);
    std::cout << "Successfully loaded " << loadedSamples << " of " << predictedSamplesCount << " samples" << std::endl;
}

samplesModule::~samplesModule() {
    std::cout << "Destructor of Samples module" << std::endl;

    for (auto& [key, set] : samples) {
        ma_sound_uninit(&set->attack);
        ma_sound_uninit(&set->sustain1);
        ma_sound_uninit(&set->sustain2);
        ma_sound_uninit(&set->release);

        delete[] set->looping.data;
        delete set;
    }

    ma_device_uninit(&device);
    ma_engine_uninit(&engine);

    std::cout << "Destructed" << std::endl;
}

void samplesModule::play(const noteSignal& signal, audioSignal&) {
    int note = signal.note;
    if (note < LOWEST_NOTE || note >= (LOWEST_NOTE + NUMBER_OF_NOTES)) return;

    for (const auto& v : this->voiceManager->getActiveVoices()) {
        int id = v.getId();
        auto it = samples.find({ id, note });
        if (it == samples.end()) continue;

        sampleSet* set = it->second;

        if (signal.on) {
            set->startLoop();
        }
        else {
            set->stopLoop();
        }
    }
}

void sampleSet::startLoop() {
    looping.loopActive = true;
    looping.cursor = looping.loopStart;
    useFirst = true;
    crossfadePos = 0.0f;

    ma_sound_stop(&sustain1);
    ma_sound_stop(&sustain2);

    ma_sound_set_volume(&sustain1, 1.0f);
    ma_sound_set_volume(&sustain2, 0.0f);

    ma_sound_start(&sustain1);
}

void sampleSet::stopLoop() {
    looping.loopActive = false;
    ma_sound_stop(&sustain1);
    ma_sound_stop(&sustain2);
}

void sampleSet::updateLooping() {   
    if (!looping.loopActive) return;

    ma_uint64 fadeStart = looping.loopEnd - looping.fadeLength;
    if (looping.cursor >= fadeStart) {
        float t = float(looping.cursor - fadeStart) / float(looping.fadeLength);

        if (useFirst) {
            if (!ma_sound_is_playing(&sustain2)) {
                ma_sound_seek_to_pcm_frame(&sustain2, looping.loopStart);
                ma_sound_start(&sustain2);
            }
            ma_sound_set_volume(&sustain1, 1.0f - t);
            ma_sound_set_volume(&sustain2, t);
        }
        else {
            if (!ma_sound_is_playing(&sustain1)) {
                ma_sound_seek_to_pcm_frame(&sustain1, looping.loopStart);
                ma_sound_start(&sustain1);
            }
            ma_sound_set_volume(&sustain2, 1.0f - t);
            ma_sound_set_volume(&sustain1, t);
        }
    }

    looping.cursor++;
    if (looping.cursor >= looping.loopEnd) {
        looping.cursor = looping.loopStart;
        useFirst = !useFirst;
    }
}