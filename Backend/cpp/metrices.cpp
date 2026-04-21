#include "../h/metrices.h"

metricBuffer::metricBuffer() : synthSignalBuffer(), modelSignalBuffer() {
    this->running = false;
}

metricBuffer::~metricBuffer() { }

void metricBuffer::init() {
    this->synthSignalBuffer.resize(SAMPLE_RATE * 10);
    this->modelSignalBuffer.resize(SAMPLE_RATE * 10);
}

void metricBuffer::push(const audioSignal& synth, const audioSignal& model) {
    if (!this->running)
        return;

    this->synthSignalBuffer.push_back(synth.getMono());
    this->modelSignalBuffer.push_back(model.getMono());
}

void metricBuffer::start() {
    running = true;
}

void metricBuffer::stop() {
    running = false;
}

void metricBuffer::clear() {
    synthSignalBuffer.clear();
    modelSignalBuffer.clear();
}

void metricBuffer::save() const {
    writeWavFloat(SYNTH_OUTPUT_PATH.string(), synthSignalBuffer);
    writeWavFloat(MODEL_OUTPUT_PATH.string(), modelSignalBuffer);
}

void metricBuffer::writeWavFloat(const std::string& path, const std::vector<float>& data) {
    std::ofstream file(path, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "ERROR: Cannot open file: " << path << std::endl;
        return;
    }

    // TODO: Magic-numbers => Some global config
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

    std::cout << "Saved: " << path << std::endl;
}