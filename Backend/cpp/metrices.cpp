#include "../h/metrices.h"

metricBuffer::metricBuffer() : samplesSignalBuffer(), synthSignalBuffer(), modelSignalBuffer() {
    this->running = false;
}

metricBuffer::~metricBuffer() { }

void metricBuffer::init() {
    this->samplesSignalBuffer.resize(SAMPLE_RATE * 10);
    this->synthSignalBuffer.resize(SAMPLE_RATE * 10);
    this->modelSignalBuffer.resize(SAMPLE_RATE * 10);
}

void metricBuffer::push(const float sample, const float synth, const float model) {
    if (!this->running)
        return;

    this->samplesSignalBuffer.push_back(sample);
    this->synthSignalBuffer.push_back(synth);
    this->modelSignalBuffer.push_back(model);
}

void metricBuffer::start() {
    running = true;
}

void metricBuffer::stop() {
    running = false;
}

void metricBuffer::clear() {
    samplesSignalBuffer.clear();
    synthSignalBuffer.clear();
    modelSignalBuffer.clear();
}

void metricBuffer::save() const {
    if (!running)
        return;

    writeWavFloat(SAMPLE_OUTPUT_PATH.string(), samplesSignalBuffer);
    writeWavFloat(SYNTH_OUTPUT_PATH.string(), synthSignalBuffer);
    writeWavFloat(MODEL_OUTPUT_PATH.string(), modelSignalBuffer);
}

void metricBuffer::writeWavFloat(const std::string& path, const std::vector<float>& data) {
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