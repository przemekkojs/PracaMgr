#pragma once

#include <vector>
#include <mutex>
#include <fstream>
#include <vector>
#include <cstdint>
#include <iostream>

#include "signal.h"
#include "paths.h"
#include "config.h"

class metricBuffer {
public:
	metricBuffer();
	~metricBuffer();

	void push(const float sample, const float synth, const float model);
	void init();
	void start();
	void stop();
	void clear();
	void save() const;

	std::vector<float>& getSamplesBuffer() { return this->samplesSignalBuffer; }
	std::vector<float>& getSynthBuffer() { return this->synthSignalBuffer; }
	std::vector<float>& getModelBuffer() { return this->modelSignalBuffer; }

private:
	static void writeWavFloat(const std::string& path, const std::vector<float>& data);

	bool running;

	std::vector<float> samplesSignalBuffer;
	std::vector<float> synthSignalBuffer;
	std::vector<float> modelSignalBuffer;
};
