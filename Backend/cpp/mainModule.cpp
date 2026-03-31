#include "../h/mainModule.h"

#include <iostream>

mainModule::mainModule() :
	voiceManager(std::make_shared<voices>()),
	samples(voiceManager),
	synth(voiceManager),
	model(voiceManager) {
	std::cout << "Initializing main module" << std::endl;

	this->initDevice();
	this->initEngine();

	unsigned int ports = midiIn.getPortCount();

	if (ports == 0)
		throw new std::exception("No devices active");

	for (unsigned int portIndex = 0; portIndex < ports; portIndex++) {
		std::cout << portIndex << " " << midiIn.getPortName(portIndex) << std::endl;
	}

	int portIndex = 1;
	midiIn.openPort(portIndex);

	std::cout << "Main module initialized." << std::endl;
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
	if (this->getSynthActive())
		synth.play(signal);

	if (this->getModelActive())
		model.play(signal);

	if (this->getSamplesActive())
		samples.play(signal);
}

void mainModule::processSample(float& outL, float& outR) {
	float synthL = 0.0f, synthR = 0.0f;
	float modelL = 0.0f, modelR = 0.0f;
	float samplesL = 0.0f, samplesR = 0.0f;

	if (this->getSynthActive())
		synth.processSample(synthL, synthR);

	if (this->getModelActive())
		model.processSample(modelL, modelR);

	if (this->getSamplesActive())
		samples.processSample(samplesL, samplesR);

	outL = synthL + modelL + samplesL;
	outR = synthR + modelR + samplesR;

	audioSignal ref{ samplesL, samplesR };
	audioSignal synthSig{ synthL, synthR };
	audioSignal modelSig{ modelL, modelR };

	bufferSynth.push(ref, synthSig);
	bufferModel.push(ref, modelSig);
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
	std::cout << "Device init" << std::endl;

	ma_device_config config = ma_device_config_init(ma_device_type_playback);

	config.playback.format = ma_format_f32;
	config.playback.channels = 2;
	config.sampleRate = 44100;
	config.dataCallback = audioCallback;
	config.pUserData = this;

	if (ma_device_init(NULL, &config, &device) != MA_SUCCESS)
		throw std::runtime_error("Device init failed");

	if (ma_device_start(&device) != MA_SUCCESS)
		throw std::runtime_error("Device start failed");

	std::cout << "Device initialised" << std::endl;
}

void mainModule::initEngine() {
	std::cout << "Engine init" << std::endl;

	if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
		throw std::runtime_error("Engine init failed!");
	}

	std::cout << "Engine initialised";
}