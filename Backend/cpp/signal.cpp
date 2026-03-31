#include "../h/signal.h"

void audioSignal::clear() {
	this->left = 0.0f;
	this->right = 0.0f;
}

void audioSignal::clearFrames() {
	this->buffer.clear();
}

void audioSignal::recordFrame() {
	float monoMix = (this->left + this->right) / 2.0f;
	this->buffer.push_back(monoMix);
}