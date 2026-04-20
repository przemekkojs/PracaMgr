#include "../h/metrices.h"

metricBuffer::metricBuffer() :
	synthSignalBuffer(),
	modelSignalBuffer(),
    queue() { }

metricBuffer::~metricBuffer() { }

void metricBuffer::push(const audioSignal& ref, const audioSignal& comp) {
    if (!running.load(std::memory_order_relaxed))
        return;

    frame f{ ref.getMono(), comp.getMono() };

    {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push_back(f);
    }

    cv.notify_one();
}

void metricBuffer::worker() {
    while (running) {
        std::vector<frame> local;

        {
            std::unique_lock<std::mutex> lock(mtx);

            cv.wait_for(lock,
                std::chrono::milliseconds(10),
                [&] { return !queue.empty() || !running; });

            local.swap(queue);
        }

        for (auto& f : local) {
            synthSignalBuffer.push_back(f.synth);
            modelSignalBuffer.push_back(f.model);
        }
    }

    std::vector<frame> local;
    {
        std::lock_guard<std::mutex> lock(mtx);
        local.swap(queue);
    }

    for (auto& f : local) {
        synthSignalBuffer.push_back(f.synth);
        modelSignalBuffer.push_back(f.model);
    }
}

void metricBuffer::start() {
    running = true;
    workerThread = std::thread(&metricBuffer::worker, this);
}

void metricBuffer::stop() {
    running = false;
    cv.notify_all();

    if (workerThread.joinable())
        workerThread.join();
}

void metricBuffer::clear() {
    std::lock_guard<std::mutex> lock(mtx);
    queue.clear();
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

    const int IEEE_FLOAT = 3;

    int sampleRate = 48000;
    short numChannels = 1;
    short bitsPerSample = 32;
    short audioFormat = IEEE_FLOAT;

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