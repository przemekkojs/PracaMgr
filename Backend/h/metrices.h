#pragma once

#include <vector>

#include "signal.h"
// Implementacja samych metryk ju¿ z poziomu pythona

constexpr int RECORDING_RATE = 48000; // Dla ViSQOLAudio jest potrzebne 48000 Hz

class metricBuffer {
public:
	metricBuffer();
	~metricBuffer();

	void record(audioSignal& ref, audioSignal& comp);
	void stopRecording(audioSignal& ref, audioSignal& comp);

	std::vector<float>& getRefBuffer() { return this->refSignalBuffer; }
	std::vector<float>& getCompBuffer() { return this->compSignalBuffer; }

protected:
	std::vector<float> refSignalBuffer;
	std::vector<float> compSignalBuffer;
};
