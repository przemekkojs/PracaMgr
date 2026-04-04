#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include <fstream>
#include <sstream>
#include <filesystem>
#include <cmath>
#include <cstdlib>

#include "../lib/dsp/delay.h"
#include "../lib/dsp/filters.h"
#include "../lib/json.hpp"

const std::filesystem::path base = std::filesystem::path(__FILE__).parent_path().parent_path();

const auto VOICES_SAMPLES_PATH = base / "./local/samples/";
const auto INSTRUMENT_PATH = base / "./local/samples/instrument";
const std::string SAMPLE_FORMAT = ".wav";

const std::string RELEASE_POSTFIX = "_release";
const std::string ATTACK_POSTFIX = "_attack";

const int LOWEST_NOTE = 48;
const int NUMBER_OF_NOTES = 24;

struct pipeParams {
    float jetDelay;
    float feedbackGain;
    float loss;
    float envSpeed;
    float noiseGain;
    float outputGain;
    float tuning;
    float pipeScale;
    float detune;
    float feedbackDamping;
    float noiseWeight;

    static pipeParams fromJson(const nlohmann::json& j) {
        pipeParams p;
        p.jetDelay = j.value("jetDelay", 20.0f);
        p.feedbackGain = j.value("feedbackGain", 0.95f);
        p.loss = j.value("loss", 0.2f);
        p.envSpeed = j.value("envSpeed", 0.001f);
        p.noiseGain = j.value("noiseGain", 0.2f);
        p.outputGain = j.value("outputGain", 0.3f);
        p.tuning = j.value("tuning", 1.0f);
        p.pipeScale = j.value("pipeScale", 1.0f);
        p.detune = j.value("detune", 0.05f);
        p.feedbackDamping = j.value("feedbackDamping", 0.8f);
        p.noiseWeight = j.value("noiseWeight", 0.3f);
        return p;
    }
};

class voice {
public:
	voice(std::string name, int id, const pipeParams& synthParams, bool active=false);	

	int getId() const { return this->id; }
	bool isActive() const { return this->active; }
	std::string getName() const { return this->name; }
    pipeParams& const getSynthParams() { return this->synthParams; }

	void setActive(bool value) { this->active = value; }

	std::string getPath() const;
	std::vector<std::string> getSamplesPath(int note) const;

private:
    pipeParams synthParams;
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
	std::vector<voice>& getVoices() { return this->container; }

private:
	void loadVoices();
	std::vector<voice> container;
};