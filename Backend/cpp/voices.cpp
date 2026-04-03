#include "../h/voices.h"
#include <iostream>

voice::voice(std::string name, int id, bool active) {
	this->name = name;
	this->id = id;
	this->active = active;
}

std::string voice::getPath() const {
	return (VOICES_SAMPLES_PATH / this->name).lexically_normal().string();
}

std::vector<std::string> voice::getSamplesPath(int note) const {
	auto attackSample = this->getPath() + "/" + std::to_string(note) + ATTACK_POSTFIX + SAMPLE_FORMAT;
	auto sustainSample = this->getPath() + "/" + std::to_string(note) + SAMPLE_FORMAT;
	auto releaseSample = this->getPath() + "/" + std::to_string(note) + RELEASE_POSTFIX + SAMPLE_FORMAT;

	std::vector<std::string> result({ attackSample, sustainSample, releaseSample });
	return result;
}

voices::voices() : container() {
	this->loadVoices();
}

void voices::loadVoices() {
	auto path = INSTRUMENT_PATH.lexically_normal();
	std::ifstream file(path);
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
