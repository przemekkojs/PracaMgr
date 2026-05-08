#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>

#include "paths.h"
#include "../lib/dsp/delay.h"
#include "../lib/dsp/filters.h"
#include "../lib/json.hpp"

const int LOWEST_NOTE = 48;
const int NUMBER_OF_NOTES = 24;

enum voiceType { FLUTE, STRING, PRINCIPAL, REED };

struct synthVoiceParams {
    float baseFrequency;
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
    int testVoiceType;

    float breathBase;
    float breathEnvAmount;
    float breathNoiseGain;
    float reedOpening;
    float reedBias;
    float reedLeak;
    float reedHysteresis;
    float reedNonlin;
    float reedNonlinEnv;
    float reedGain;
    float buzzGain;
    float attackNoiseGain;
    float attackEnvThreshold;
    float resonanceFreq;
    float resonanceGain;
    float feedbackNonlin;
    float jetDelayOffset;

    static synthVoiceParams fromJson(const nlohmann::json& j) {
        synthVoiceParams p;

        p.baseFrequency = j.value("baseFrequency", 440.0f);
        p.reflection = j.value("reflection", 0.5f);
        p.excitationGain = j.value("excitationGain", 0.3f);
        p.noiseGain = j.value("noiseGain", 0.0f);
        p.scale = j.value("scale", 1.0f);
        p.jetGain = j.value("jetGain", 0.0f);
        p.jetLength = j.value("jetLength", 0.5f);
        p.loopFeedbackGain = j.value("loopFeedbackGain", 0.9f);
        p.jetLowpassCoeff = j.value("jetLowpassCoeff", 0.5f);
        p.lowpassCoeff = j.value("lowpassCoeff", 0.9f);
        p.nonlinearCoeff = j.value("nonlinearCoeff", 1.0f);
        p.lossFilterCoeff = j.value("lossFilterCoeff", 0.3f);

        p.breathBase = j.value("breathBase", 0.1f);
        p.breathEnvAmount = j.value("breathEnvAmount", 0.9f);
        p.breathNoiseGain = j.value("breathNoiseGain", 0.01f);
        p.reedOpening = j.value("reedOpening", 0.0003f);
        p.reedBias = j.value("reedBias", 0.4f);
        p.reedLeak = j.value("reedLeak", 0.001f);
        p.reedHysteresis = j.value("reedHysteresis", 0.7f);
        p.reedNonlin = j.value("reedNonlin", 4.0f);
        p.reedNonlinEnv = j.value("reedNonlinEnv", 0.5f);
        p.reedGain = j.value("reedGain", 1.2f);
        p.buzzGain = j.value("buzzGain", 0.02f);
        p.attackNoiseGain = j.value("attackNoiseGain", 0.1f);
        p.attackEnvThreshold = j.value("attackEnvThreshold", 0.1f);
        p.resonanceFreq = j.value("resonanceFreq", 3000.0f);
        p.resonanceGain = j.value("resonanceGain", 0.3f);
        p.feedbackNonlin = j.value("feedbackNonlin", 2.0f);
        p.jetDelayOffset = j.value("jetDelayOffset", 1.0f);

        p.testVoiceType = j.value("testVoiceType", 1);

        return p;
    }

    static voiceType getVoiceType(char s) {
        switch (s) {
            case 'P': return voiceType::PRINCIPAL;
            case 'F': return voiceType::FLUTE;
            case 'S': return voiceType::STRING;
            case 'R': return voiceType::REED;
            default: throw std::exception("Invalid voice type");
        }
    }
};

class voice {
public:
	voice(std::string name, int id, voiceType vType, const synthVoiceParams& synthParams, bool active=false);

	int getId() const { return this->id; }
	bool isActive() const { return this->active; }
	std::string getName() const { return this->name; }
    synthVoiceParams& const getSynthParams() { return this->synthParams; }
    voiceType& const getVoiceType() { return this->vType; }

	void setActive(bool value) { this->active = value; }

	std::string getPath() const;
	std::vector<std::string> getSamplesPath(int note) const;

private:
    voiceType vType;
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