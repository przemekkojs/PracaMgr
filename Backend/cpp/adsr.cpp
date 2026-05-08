#include "../h/adsr.h"

float ADSR::process() {
    switch (state) {
    case AdsrState::ATTACK:
        value += attackRate;

        if (value >= 1.0f) {
            value = 1.0f;
            state = AdsrState::DECAY;
        }

        break;
    case AdsrState::DECAY:
        value -= decayRate;

        if (value <= sustainLevel) {
            value = sustainLevel;
            state = AdsrState::SUSTAIN;
        }

        break;
    case AdsrState::SUSTAIN:
        break;
    case AdsrState::RELEASE:
        value -= releaseRate;

        if (value <= 0.0f) {
            value = 0.0f;
            state = AdsrState::IDLE;
        }

        break;
    default: break;
    }

    return value;
}

void ADSR::calculateRate(float& what, float seconds) const {
    what = (1.0f / (seconds * SAMPLE_RATE));
}