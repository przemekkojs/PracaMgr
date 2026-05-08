#pragma once

#include "config.h"

enum class AdsrState { IDLE, ATTACK, DECAY, SUSTAIN, RELEASE };

class ADSR {
public:
	float process();
	void noteOn() { state = AdsrState::ATTACK; value = 0.0f; }
	void noteOff() { state = AdsrState::RELEASE; }
	bool isActive() const { return state != AdsrState::IDLE; }

	void setAttack(float seconds) { this->calculateRate(this->attackRate, seconds); }
	void setDecay(float seconds) { this->calculateRate(this->decayRate, seconds); }
	void setRelease(float seconds) { this->calculateRate(this->releaseRate, seconds); }
	void setSustain(float seconds) { this->calculateRate(this->sustainLevel, seconds); }

	void calculateRate(float& what, float seconds) const;

private:
	AdsrState state = AdsrState::IDLE;
	float value = 0.0f;
	float attackRate = 0.001f;
	float decayRate = 0.0005f;
	float releaseRate = 0.0008f;
	float sustainLevel = 0.7f;
};