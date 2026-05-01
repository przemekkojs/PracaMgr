#pragma once

#include "synthModule.h"
#include "voices.h"
#include "paths.h"

#include <vector>
#include <fstream>
#include <cstdint>
#include <iostream>

class testModule {
public:
	testModule();

	void makeSynth() { this->makeArtificial(TEST_OUTPUT_PATH_COMP_SYNTH.string()); }
	void makeModel() { this->makeArtificial(TEST_OUTPUT_PATH_COMP_MODEL.string()); }
	void makeSample(std::string basePath);

private:
	void makeArtificial(const std::string& path);
	void pruneFile(const std::string& inPath, const std::string& outPath);
    void writeWavFloat(const std::string& path, const std::vector<float>& data);
};
