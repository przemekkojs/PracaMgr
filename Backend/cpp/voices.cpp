#include "../h/voices.h"

voice::voice(std::string name, int id, bool active) {
	this->name = name;
	this->id = id;
	this->active = active;
}

std::string voice::getPath() const {
	return VOICES_SAMPLES_PATH + this->name + "/";
}

std::vector<std::string> voice::getSamplesPath(int note) const {
	std::string attackSample = this->getPath() + std::to_string(note) + ATTACK_POSTFIX + SAMPLE_FORMAT;
	std::string mainSample = this->getPath() + std::to_string(note) + SAMPLE_FORMAT;
	std::string releaseSample = this->getPath() + std::to_string(note) + RELEASE_POSTFIX + SAMPLE_FORMAT;

	std::vector<std::string> result({ attackSample, mainSample, releaseSample });
	return result;
}


voices::voices() : container() {
	std::cout << "Voices manager init" << std::endl;
	this->loadVoices();
}

void voices::loadVoices() {
	std::ifstream file(INSTRUMENT_PATH);
	std::string line;

	while (std::getline(file, line)) {
		std::stringstream ss(line);
		std::string cell;
		std::vector<std::string> row;

		while (std::getline(ss, cell, ',')) {
			row.push_back(cell);
		}

		int id = std::stoi(row[0]);
		std::string name = row[1];

		std::transform(name.begin(), name.end(), name.begin(), ::tolower);
		std::replace(name.begin(), name.end(), ' ', '-');
		voice v = voice(name, id, false);
		this->container.push_back(v);
	}
}

bool voices::setActive(int id, bool value) {
	std::cout << "Voice " << id << ": " << value << std::endl;

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

std::vector<voice> voices::getActiveVoices() {
	std::vector<voice> result;

	for (auto& v : this->container) {
		if (v.isActive()) {
			result.push_back(v);
		}
	}

	return result;
}
