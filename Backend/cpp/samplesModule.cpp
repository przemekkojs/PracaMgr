#include "../h/samplesModule.h"

#include <iostream>

samplesModule::samplesModule(std::shared_ptr<voices> voiceManager) : module(std::move(voiceManager)) {
	ma_engine_init(NULL, &this->engine);
	
	for (auto& v : this->voiceManager->getVoices()) {
		for (int note = LOWEST_NOTE; note < (LOWEST_NOTE + NUMBER_OF_NOTES); note++) {
			std::vector<std::string> paths = v.getSamplesPath(note);

			std::string attackPath = paths[0];
			std::string durationPath = paths[1];
			std::string releasePath = paths[2];

			SampleSet set = SampleSet();

			ma_sound_init_from_file(&engine, attackPath.c_str(), 0, NULL, NULL, &set.attack);
			ma_sound_init_from_file(&engine, durationPath.c_str(), 0, NULL, NULL, &set.sustain);
			ma_sound_init_from_file(&engine, releasePath.c_str(), 0, NULL, NULL, &set.release);

			samples[{v.getId(), note}] = std::move(set);
		}
	}
}

samplesModule::~samplesModule() {
	for (auto& [key, set] : samples) {
		ma_sound_uninit(&set.attack);
		ma_sound_uninit(&set.sustain);
		ma_sound_uninit(&set.release);
	}

	ma_engine_uninit(&engine);
}

void samplesModule::play(const noteSignal& signal, audioSignal& output) {
	int note = signal.note;

	if (note < LOWEST_NOTE || note >(LOWEST_NOTE + NUMBER_OF_NOTES))
		return;
	
	for (const auto& v : this->voiceManager->getActiveVoices()) {
		int id = v.getId();

		auto it = samples.find({ id, note });
		if (it == samples.end()) continue;

		ma_sound_start(&it->second.sustain);
	}
}