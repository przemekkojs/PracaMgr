#include "../h/mainModule.h"

#include <iostream>

mainModule::mainModule() : voiceManager(std::make_shared<voices>()), samples(voiceManager), synth(voiceManager), model(voiceManager) {
	unsigned int ports = midiIn.getPortCount();

	if (ports == 0)
		throw new std::exception("No devices active");

	midiIn.openPort(MIDI_PORT_ID);

	this->initDevice();
	this->initEngine();

	this->setSamplesActive(false);
	this->setModelActive(false);
	this->setSynthActive(false);	
}

mainModule::~mainModule() {
	ma_device_uninit(&device);
	ma_engine_uninit(&engine);
}

noteSignal mainModule::getSignal() {
	std::vector<unsigned char> message;
	double timestamp = midiIn.getMessage(&message);

	if (message.size() >= 3) {
		unsigned char note = message[1];
		unsigned char channel = message[0] & 0x0F;
		unsigned char on = (message[0] & 0xF0) == 0x90;

		return noteSignal(note, channel, on);
	}

	return EMPTY_NOTE_SIGNAL;
}

void mainModule::play(noteSignal& signal) {
	bool synthActive = this->getSynthActive();
	bool modelActive = this->getModelActive();
	bool samplesActive = this->getSamplesActive();

	if (signal.on) {
		if (samplesActive && synthActive) {
			buffer.clear();
			buffer.start();
		}
	}
	else {
		buffer.stop();
	}

	if (synthActive)
		synth.play(signal);

	if (modelActive)
		model.play(signal);

	if (samplesActive)
		samples.play(signal);
}

void mainModule::processSample(float& outL, float& outR) {
	float masterGain = 1.0f;
	float synthL = 0.0f;
	float synthR = 0.0f;
	float modelL = 0.0f;
	float modelR = 0.0f;
	float samplesL = 0.0f;
	float samplesR = 0.0f;

	if (this->getSynthActive())
		synth.processSample(synthL, synthR);

	if (this->getModelActive())
		model.processSample(modelL, modelR);

	if (this->getSamplesActive())
		samples.processSample(samplesL, samplesR);

	outL = (synthL + modelL + samplesL) * masterGain;
	outR = (synthR + modelR + samplesR) * masterGain;
	outL = std::clamp(outL, -1.0f, 1.0f);
	outR = std::clamp(outR, -1.0f, 1.0f);

	audioSignal ref { samplesL, samplesR };
	audioSignal synthSig { synthL, synthR };
	audioSignal modelSig { modelL, modelR };

	buffer.push(synthSig, modelSig);
}

void mainModule::audioCallback(ma_device* device, void* output, const void*, ma_uint32 frameCount) {
	auto* self = (mainModule*)device->pUserData;
	float* out = (float*)output;

	for (ma_uint32 i = 0; i < frameCount; i++) {
		float outL = 0.0f;
		float outR = 0.0f;

		self->processSample(outL, outR);

		out[i * 2 + 0] = outL;
		out[i * 2 + 1] = outR;
	}
}

void mainModule::initDevice() {
	ma_device_config config = ma_device_config_init(ma_device_type_playback);

	config.playback.format = ma_format_f32;
	config.playback.channels = 2;
	config.sampleRate = 48000;
	config.dataCallback = audioCallback;
	config.pUserData = this;

	if (ma_device_init(NULL, &config, &device) != MA_SUCCESS)
		throw std::runtime_error("Device init failed");

	if (ma_device_start(&device) != MA_SUCCESS)
		throw std::runtime_error("Device start failed");
}

void mainModule::initEngine() {
	if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
		throw std::runtime_error("Engine init failed!");
	}
}

std::map<int, std::string> mainModule::getVoicesNames() {
	std::map<int, std::string> result;

	for (auto& v : this->voiceManager->getVoices()) {
		result.emplace(v.getId(), v.getName());
	}

	return result;
}