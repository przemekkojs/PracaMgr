#include "../h/samplesModule.h"

samplesModule::samplesModule(std::shared_ptr<voices> voiceManager, int maxPolyphony) : module(std::move(voiceManager)) {
    std::cout << "Samples module init" << std::endl;

    this->maxPolyphony = maxPolyphony;

    if (ma_engine_init(NULL, &this->engine) != MA_SUCCESS) {
        throw std::runtime_error("Engine init failed!");
    }

    this->initEngine();
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
        }
    }

    ma_engine_set_volume(&engine, 1.0f);
    std::cout << "Successfully loaded " << loadedSamples << " of " << predictedSamplesCount << " samples" << std::endl;
}

samplesModule::~samplesModule() {
    std::cout << "Destructor of Samples module" << std::endl;

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

        if (it == samples.end()) {
            continue;
        }
    }
}