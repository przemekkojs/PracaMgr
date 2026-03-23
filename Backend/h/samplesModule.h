#pragma once

#include "module.h"
#include "../lib/miniaudio.h"

#include <map>
#include <vector>
#include <memory>


class samplesModule : public module {
public:
    samplesModule(std::shared_ptr<voices> voiceManager, int maxPolyphony = 1024);
    ~samplesModule();

    void play(const noteSignal& signal, audioSignal& output) override;
    void initEngine();

private:
    std::map<std::pair<int, int>, int> samples;

    int maxPolyphony;
};
