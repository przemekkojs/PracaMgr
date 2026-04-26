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

	void makeSynth();
	void makeSample(std::string basePath);

private:
	void pruneFile(const std::string& inPath, const std::string& outPath);
    void writeWavFloat(const std::string& path, const std::vector<float>& data);
};
