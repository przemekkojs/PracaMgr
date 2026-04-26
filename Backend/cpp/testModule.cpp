#include "../h/testModule.h"

testModule::testModule() { }

void testModule::makeSynth() {	
	std::ifstream f(TEST_PARAMS_PATH);
	nlohmann::json j;
	f >> j;

	synthVoiceParams baseParams =
	synthVoiceParams::fromJson(j);
	synthPipeParams params;
	synthPipe pipe;
	
	const float duration = 10.0f;
	float freq = baseParams.baseFrequency * std::pow(2.0f, -9 / 12.0f) * baseParams.scale;
	float filterDelayComp = 1.0f;

	params.baseParams = baseParams;
	params.frequency = freq;
	params.delaySamples = (SAMPLE_RATE / freq) - filterDelayComp;
	params.jetDelaySamples = params.delaySamples * baseParams.jetLength;
	params.jetDelaySamples = std::clamp(params.jetDelaySamples, 2.0f, params.delaySamples * 0.9f);

	pipe.load(params);   

    std::vector<float> data;
    float out = 0.0f;
    int size = static_cast<int>(SAMPLE_RATE * duration);
    data.reserve(size);

    pipe.noteOn();

    for (int i = 0; i < size; i++) {
        data.push_back(pipe.process());
    }

    this->writeWavFloat(TEST_OUTPUT_PATH_COMP.string(), data);
    this->pruneFile(TEST_OUTPUT_PATH_COMP.string(), TEST_OUTPUT_PATH_COMP.string());
}

void testModule::makeSample(std::string basePath) {
    this->pruneFile(basePath, TEST_OUTPUT_PATH_REF.string());
}

void testModule::writeWavFloat(const std::string& path, const std::vector<float>& data) {
    std::ofstream file(path, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "ERROR: Cannot open file: " << path << std::endl;
        return;
    }

    const int sampleRate = SAMPLE_RATE;
    const short numChannels = OUT_CHANNELS;
    const short bitsPerSample = BIT_DEPTH;
    const short audioFormat = IEEE_FLOAT;

    int byteRate = sampleRate * numChannels * bitsPerSample / 8;
    short blockAlign = numChannels * bitsPerSample / 8;
    int dataSize = data.size() * blockAlign;
    int chunkSize = 36 + dataSize;

    file.write("RIFF", 4);
    file.write(reinterpret_cast<const char*>(&chunkSize), 4);
    file.write("WAVE", 4);

    file.write("fmt ", 4);
    int subchunk1Size = 16;
    file.write(reinterpret_cast<const char*>(&subchunk1Size), 4);
    file.write(reinterpret_cast<const char*>(&audioFormat), 2);
    file.write(reinterpret_cast<const char*>(&numChannels), 2);
    file.write(reinterpret_cast<const char*>(&sampleRate), 4);
    file.write(reinterpret_cast<const char*>(&byteRate), 4);
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);
    file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&dataSize), 4);

    file.write(reinterpret_cast<const char*>(data.data()), dataSize);
}

void testModule::pruneFile(const std::string& inPath, const std::string& outPath) {
    std::ifstream in(inPath, std::ios::binary);
    if (!in) {
        std::cerr << "Cannot open input file\n";
        return;
    }

    struct WavHeader {
        char riff[4];
        uint32_t chunkSize;
        char wave[4];
    } header;

    in.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (std::string(header.riff, 4) != "RIFF" ||
        std::string(header.wave, 4) != "WAVE") {
        std::cerr << "Not a valid WAV file\n";
        return;
    }

    uint16_t audioFormat = 0;
    uint16_t numChannels = 0;
    uint32_t sampleRate = 0;
    uint16_t bitsPerSample = 0;
    uint32_t dataOffset = 0;
    uint32_t dataSize = 0;

    while (in && !dataOffset) {
        char chunkId[4];
        uint32_t chunkSize = 0;

        in.read(chunkId, 4);
        in.read(reinterpret_cast<char*>(&chunkSize), 4);

        std::string id(chunkId, 4);

        if (id == "fmt ") {
            in.read(reinterpret_cast<char*>(&audioFormat), 2);
            in.read(reinterpret_cast<char*>(&numChannels), 2);
            in.read(reinterpret_cast<char*>(&sampleRate), 4);
            in.seekg(6, std::ios::cur);
            in.read(reinterpret_cast<char*>(&bitsPerSample), 2);
            in.seekg(chunkSize - 16, std::ios::cur);
        }
        else if (id == "data") {
            dataOffset = static_cast<uint32_t>(in.tellg());
            dataSize = chunkSize;
            break;
        }
        else {
            in.seekg(chunkSize, std::ios::cur);
        }
    }

    if (!dataOffset) {
        std::cerr << "No data chunk found\n";
        return;
    }

    const uint32_t frameSize = numChannels * (bitsPerSample / 8);
    const uint32_t totalSamples = dataSize / frameSize;
    const uint32_t startSamples = 2 * sampleRate;
    const uint32_t targetSamples = 5 * sampleRate;

    if (totalSamples <= startSamples) {
        std::cerr << "File too short\n";
        return;
    }

    const uint32_t availableSamples = totalSamples - startSamples;
    const uint32_t samplesToCopy = std::min(targetSamples, availableSamples);
    const uint32_t startOffsetBytes = startSamples * frameSize;
    const uint32_t bytesToCopy = samplesToCopy * frameSize;

    in.seekg(dataOffset + startOffsetBytes, std::ios::beg);

    std::vector<char> buffer(bytesToCopy);
    in.read(buffer.data(), bytesToCopy);
    in.close();

    std::ofstream out(outPath, std::ios::binary);

    uint32_t newDataSize = bytesToCopy;
    uint32_t chunkSize = 36 + newDataSize;

    out.write("RIFF", 4);
    out.write(reinterpret_cast<char*>(&chunkSize), 4);
    out.write("WAVE", 4);

    uint32_t fmtSize = 16;
    audioFormat = 3;

    out.write("fmt ", 4);
    out.write(reinterpret_cast<char*>(&fmtSize), 4);
    out.write(reinterpret_cast<char*>(&audioFormat), 2);
    out.write(reinterpret_cast<char*>(&numChannels), 2);
    out.write(reinterpret_cast<char*>(&sampleRate), 4);

    uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
    uint16_t blockAlign = numChannels * (bitsPerSample / 8);

    out.write(reinterpret_cast<char*>(&byteRate), 4);
    out.write(reinterpret_cast<char*>(&blockAlign), 2);
    out.write(reinterpret_cast<char*>(&bitsPerSample), 2);

    out.write("data", 4);
    out.write(reinterpret_cast<char*>(&newDataSize), 4);
    out.write(buffer.data(), newDataSize);

    out.close();
}