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

struct synthVoiceParams {
    float baseFrequency;
    float sampleRate;
    float reflection;
    float excitationGain;
    float noiseGain;
    float jetGain;
    float scale;
    float jetLength;
    float loopFeedbackGain;
    
    float jetLowpassCoeff;
    float lowpassCoeff;
    float nonlinearCoeff;
    float lossFilterCoeff;

    static synthVoiceParams fromJson(const nlohmann::json& j) {
        synthVoiceParams p;
        p.baseFrequency = j.value("baseFrequency", 440.0f);
        p.sampleRate = j.value("sampleRate", 48000.0f);
        p.reflection = j.value("reflection", 0.5f);
        p.excitationGain = j.value("excitationGain", 0.3f);
        p.noiseGain = j.value("noiseGain", 0.0f);
        p.scale = j.value("scale", 1.0f);
        p.jetGain = j.value("jetGain", 0.0f);
        p.jetLength = j.value("jLength", 0.5f);
        p.jetLowpassCoeff = j.value("jetLowpassCoeff", 0.5f);
        p.lowpassCoeff = j.value("lowpassCoeff", 0.9f);
        p.nonlinearCoeff = j.value("nonlinearCoeff", 1.0f);
        p.lossFilterCoeff = j.value("lossFilterCoeff", 0.3f);
        p.loopFeedbackGain = j.value("loopFeedbackGain", 0.9f);
        return p;
    }
};

class voice {
public:
	voice(std::string name, int id, const synthVoiceParams& synthParams, bool active=false);

	int getId() const { return this->id; }
	bool isActive() const { return this->active; }
	std::string getName() const { return this->name; }
    synthVoiceParams& const getSynthParams() { return this->synthParams; }

	void setActive(bool value) { this->active = value; }

	std::string getPath() const;
	std::vector<std::string> getSamplesPath(int note) const;

private:
    synthVoiceParams synthParams;
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