#include "../h/samplesModule.h"

#include <iostream>

samplesModule::samplesModule(std::shared_ptr<voices> voiceManager) : module(std::move(voiceManager)) {
	for (auto& v : this->voiceManager.get()->getVoices()) {
		for (int note = 0; note < 61; note++) {
			std::vector<std::string> paths = v.getSamplesPath(note);

			std::string attackPath = paths[0];
			std::string durationPath = paths[1];
			std::string releasePath = paths[2];

			// Tutaj trzeba zrobiæ ³adowanie plików .wav	
			this->samples[std::make_pair(v.getId(), note)] = durationPath;
		}
	}
}

void samplesModule::play(const noteSignal& signal, audioSignal& output) {
	int note = signal.note;
	
	for (auto& v : this->voiceManager.get()->getActiveVoices()) {		
		int id = v.getId();
		std::cout << this->samples[std::make_pair(id, note)] << std::endl;
	}
}