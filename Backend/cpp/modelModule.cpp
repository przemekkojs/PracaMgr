#include "../h/modelModule.h"

modelModule::modelModule(std::shared_ptr<voices> voiceManager) : module(voiceManager) { }

audioSignal& modelModule::play(noteSignal& signal) {
	return EMPTY_AUDIO_SIGNAL;
}