#include "../h/module.h"

module::module(std::shared_ptr<voices> voiceManager) {
	this->voiceManager = voiceManager;
}