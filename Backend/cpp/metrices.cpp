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

void metricBuffer::save() {
    // Sta³e do zapisu to:
    // - SYNTH_OUTPUT_PATH (synthSignalBuffer - vector<float>)
    // - MODEL_OUTPUT_PATH (modelSignalBuffer - vector<float>)
    // To ju¿ s¹ œcie¿ki do plików .wav, nie folder - plik trzeba po prostu nadpisaæ
    // Czyli robimy plik wav na podstawie surowych danych float
    // Trzeba zapisywaæ w mono, 48 kHz sample rate
}