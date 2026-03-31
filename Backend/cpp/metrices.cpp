#include "../h/metrices.h"

metricBuffer::metricBuffer() : refSignalBuffer(), compSignalBuffer() {

}

metricBuffer::~metricBuffer() {

}

void metricBuffer::record(audioSignal& ref, audioSignal& comp) {	
	ref.clearFrames();
	comp.clearFrames();
}

void metricBuffer::stopRecording(audioSignal& ref, audioSignal& comp) {
	this->refSignalBuffer.clear();
	this->compSignalBuffer.clear();

	for (auto f : ref.buffer)
		this->refSignalBuffer.push_back(f);

	for (auto f : comp.buffer)
		this->compSignalBuffer.push_back(f);
}