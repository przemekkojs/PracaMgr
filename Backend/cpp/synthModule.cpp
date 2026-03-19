#include "../h/synthModule.h"

synthModule::synthModule(std::shared_ptr<voices> voiceManager) : module(voiceManager) { }

audioSignal& synthModule::play(noteSignal& signal) {

}