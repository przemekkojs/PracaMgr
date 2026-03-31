#include "../h/metrices.h"

metricBuffer::metricBuffer() :
	refSignalBuffer(),
	compSignalBuffer(),
    queue() { }

metricBuffer::~metricBuffer() {

}

void metricBuffer::push(const audioSignal& ref, const audioSignal& comp) {
    std::cout << "a";

    if (!running.load(std::memory_order_relaxed))
        return;

    std::cout << "b";

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
            refSignalBuffer.push_back(f.ref);
            compSignalBuffer.push_back(f.comp);
        }
    }

    std::vector<frame> local;
    {
        std::lock_guard<std::mutex> lock(mtx);
        local.swap(queue);
    }

    for (auto& f : local) {
        refSignalBuffer.push_back(f.ref);
        compSignalBuffer.push_back(f.comp);
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
    refSignalBuffer.clear();
    compSignalBuffer.clear();
}