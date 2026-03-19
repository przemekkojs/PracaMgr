#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

const std::string VOICES_SAMPLES_PATH = "./local/samples/";
const std::string INSTRUMENT_PATH = "./local/samples/instrument";
const std::string SAMPLE_FORMAT = ".wav";

const std::string RELEASE_POSTFIX = "_release";
const std::string ATTACK_POSTFIX = "_attack";

const int LOWEST_NOTE = 48;
const int NUMBER_OF_NOTES = 24;

class voice {
public:
	voice(std::string name, int id, bool active=false);	

	int getId() const { return this->id; }
	bool isActive() const { return this->active; }
	std::string getName() const { return this->name; }

	void setActive(bool value) { this->active = value; }

	std::string getPath() const;
	std::vector<std::string> getSamplesPath(int note) const;

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
	std::vector<voice> getActiveVoices();
	const std::vector<voice>& getVoices() const { return this->container; }

private:
	void loadVoices();
	std::vector<voice> container;
};