#pragma once

#include <vector>
#include <mutex>

#include "signal.h"
#include "paths.h"

struct frame {
	float synth;
	float model;
};

class metricBuffer {
public:
	metricBuffer();
	~metricBuffer();

	void push(const audioSignal& ref, const audioSignal& comp);
	void start();
	void stop();
	void worker();
	void clear();
	void save();

	std::vector<float>& getRefBuffer() { return this->synthSignalBuffer; }
	std::vector<float>& getCompBuffer() { return this->modelSignalBuffer; }

private:
	std::thread workerThread;
	std::mutex mtx;
	std::condition_variable cv;
	std::atomic<bool> running{ false };
	std::vector<frame> queue;

	std::vector<float> synthSignalBuffer;
	std::vector<float> modelSignalBuffer;
};
