#pragma once

#include <vector>
#include <mutex>

#include "signal.h"
// Implementacja samych metryk ju¿ z poziomu pythona

constexpr int RECORDING_RATE = 48000; // Dla ViSQOLAudio jest potrzebne 48000 Hz

struct frame {
	float ref;
	float comp;
};

class metricBuffer {
public:
	metricBuffer();
	~metricBuffer();

	void push(const audioSignal& ref, const audioSignal& comp);
	void start();
	void stop();
	void worker();

	std::vector<float>& getRefBuffer() { return this->refSignalBuffer; }
	std::vector<float>& getCompBuffer() { return this->compSignalBuffer; }

private:
	std::thread workerThread;
	std::mutex mtx;
	std::condition_variable cv;
	std::atomic<bool> running{ false };
	std::vector<frame> queue;

	std::vector<float> refSignalBuffer;
	std::vector<float> compSignalBuffer;
};
