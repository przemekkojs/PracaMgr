#include "../h/voices.h"

voice::voice(std::string name, int id, bool active=false) {
	this->name = name;
	this->id = id;
	this->active = active;
}

std::string voice::getPath() const {
	return VOICES_SAMPLES_PATH + this->name + "/";
}

std::vector<std::string> voice::getSamplesPath(int note) {
	std::string attackSample = this->getPath() + (char)note + ATTACK_POSTFIX + SAMPLE_FORMAT;
	std::string mainSample = this->getPath() + (char)note + SAMPLE_FORMAT;
	std::string releaseSample = this->getPath() + (char)note + RELEASE_POSTFIX + SAMPLE_FORMAT;

	std::vector<std::string> result({ attackSample, mainSample, releaseSample });
	return result;
}


voices::voices() : container() {
	this->loadVoices();
}

void voices::loadVoices() {
	// Tutaj bêdzie ³adowanie plików .wav do pamiêci
	// 1. Przeczytanie pliku instrument z local/samples
	// 2. Dotarcie do folderów
	// 3. Za³adowanie próbek attack, duration i release
}

bool voices::setActive(int id, bool value) {
	for (auto& v : this->container) {
		if (v.getId() == id) {
			v.setActive(value);
			return true;
		}
	}

	return false;
}

std::vector<std::string> voices::getActiveSamplesPaths() {
	std::vector<std::string> result;

	for (auto& v : this->container) {
		if (v.isActive()) {
			result.push_back(v.getPath());
		}
	}

	return result;
}

std::vector<voice&> voices::getActiveVoices() {
	std::vector<voice&> result;

	for (auto& v : this->container) {
		if (v.isActive()) {
			result.push_back(v);
		}
	}

	return result;
}
