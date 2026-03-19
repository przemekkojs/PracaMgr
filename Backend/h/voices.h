#pragma once

#include <string>
#include <vector>

const std::string VOICES_SAMPLES_PATH = "local/samples";

class voice {
public:
	voice(std::string name, int id);

	std::string getName() { return this->name; }
	std::string getPath();
	std::string getSamplePath(int note);
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
	void setActive(int id, bool value);
	std::vector<std::string>& getActiveSamplesPaths();
	std::vector<voice>& getActiveVoices();

private:
	void loadVoices();
	std::vector<voice> container;
};