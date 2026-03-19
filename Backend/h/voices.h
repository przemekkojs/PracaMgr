#pragma once

#include <string>
#include <vector>
#include <algorithm>

const std::string VOICES_SAMPLES_PATH = "local/samples/";
const std::string SAMPLE_FORMAT = ".wav";

const std::string RELEASE_POSTFIX = "_release";
const std::string ATTACK_POSTFIX = "_attack";

class voice {
public:
	voice(std::string name, int id, bool active=false);

	std::string getName() { return this->name; }
	std::string getPath() const;
	std::vector<std::string> getSamplesPath(int note);
	int getId() { return this->id; }
	bool isActive() { return this->active; }

	void setActive(bool value) { this->active = value; }

private:
	std::string name;
	bool active;
	int id;
};

class voices {
public:
	voices();
	bool setActive(int id, bool value);
	std::vector<std::string> getActiveSamplesPaths();
	std::vector<voice&> getActiveVoices();

private:
	void loadVoices();
	std::vector<voice> container;
};