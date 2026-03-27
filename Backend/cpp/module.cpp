#include "../h/module.h"

module::module(std::shared_ptr<voices> voiceManager) {
	this->voiceManager = voiceManager;
	this->active = false;
}

void module::setActive(bool value) {
	bool prevValue = this->active;
	this->active = value;

	if (value != prevValue) {
		if (value)
			this->load();
		else
			this->unload();
	}
}