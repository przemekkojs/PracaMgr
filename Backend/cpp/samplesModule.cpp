#include "../h/samplesModule.h"

#include <iostream>

samplesModule::samplesModule(std::shared_ptr<voices> voiceManager) : module(std::move(voiceManager)) {
	std::cout << "Samples module init" << std::endl;
	ma_result initResult = ma_engine_init(NULL, &this->engine);	

	if (initResult != MA_SUCCESS) {
		std::cout << "Engine init failed!" << std::endl;
		throw std::exception("Engine init failed!");
	}

	this->initEngine();
}

void samplesModule::initEngine() {
	std::cout << "Samples engine init" << std::endl;
	int loadedSamples = 0;
	const int predictedSamplesCount = this->voiceManager->getVoices().size() * NUMBER_OF_NOTES;

	for (auto& v : this->voiceManager->getVoices()) {
		for (int note = LOWEST_NOTE; note < (LOWEST_NOTE + NUMBER_OF_NOTES); note++) {
			std::vector<std::string> paths = v.getSamplesPath(note);

			std::string attackPath = paths[0];
			std::string sustainPath = paths[1];
			std::string releasePath = paths[2];

			sampleSet* set = new sampleSet();

			if (ma_sound_init_from_file(&engine, attackPath.c_str(), 0, NULL, NULL, &set->attack) != MA_SUCCESS) {
				std::cout << "Failed to load attack: " << attackPath << std::endl;
				continue;
			}

			if (ma_sound_init_from_file(&engine, sustainPath.c_str(), 0, NULL, NULL, &set->sustain) != MA_SUCCESS) {
				ma_sound_uninit(&set->attack);

				std::cout << "Failed to load sustain: " << sustainPath << std::endl;
				continue;
			}

			if (ma_sound_init_from_file(&engine, releasePath.c_str(), 0, NULL, NULL, &set->release) != MA_SUCCESS) {
				ma_sound_uninit(&set->attack);
				ma_sound_uninit(&set->sustain);

				std::cout << "Failed to load release: " << releasePath << std::endl;
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
	std::cout << "Destructor of Samples module";

	for (auto& [key, set] : samples) {
		ma_sound_uninit(&set->attack);
		ma_sound_uninit(&set->sustain);
		ma_sound_uninit(&set->release);

		delete set;
	}

	ma_engine_uninit(&engine);

	std::cout << "Destructed";
}

void samplesModule::play(const noteSignal& signal, audioSignal& output) {
	int note = signal.note;

	if (note < LOWEST_NOTE || note >(LOWEST_NOTE + NUMBER_OF_NOTES))
		return;
	
	for (const auto& v : this->voiceManager->getActiveVoices()) {
		int id = v.getId();
		auto it = samples.find({ id, note });

		if (it == samples.end())
			continue;

		if (signal.on)
			ma_sound_start(&it->second->sustain);
		else
			ma_sound_stop(&it->second->sustain);
	}
}