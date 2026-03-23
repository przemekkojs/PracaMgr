#include "../h/samplesModule.h"

#include <iostream>

ma_result LoopingSample_read(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead) {
	LoopingSample* s = (LoopingSample*)pDataSource;	
	ma_uint64 framesWritten = 0;
	ma_uint64 loopLength = s->loopEnd - s->loopStart;
	float* out = (float*)pFramesOut;

	while (framesWritten < frameCount) {
		ma_uint64 frame = s->cursor;

		for (ma_uint32 ch = 0; ch < s->channels; ch++) {
			float sample = 0.0f;

			if (s->loopActive) {
				if (frame >= s->loopEnd - s->fadeLength) {
					ma_uint64 fadePos = frame - (s->loopEnd - s->fadeLength);
					float t = (float)fadePos / (float)s->fadeLength;

					float tail = s->data[frame * s->channels + ch] * (1.0f - t);

					ma_uint64 headFrame = s->loopStart + fadePos;
					if (headFrame >= s->loopEnd)
						headFrame -= (s->loopEnd - s->loopStart);
					float head = s->data[headFrame * s->channels + ch] * t;

					sample = tail + head;
				}
				else {
					sample = s->data[frame * s->channels + ch];
				}
			}
			else {
				if (frame < s->frameCount - s->fadeLength) {
					sample = s->data[frame * s->channels + ch];
				}
				else {
					ma_uint64 fadePos = frame - (s->frameCount - s->fadeLength);
					float t = (float)fadePos / (float)s->fadeLength;
					sample = s->data[frame * s->channels + ch] * (1.0f - t);
				}
			}

			out[framesWritten * s->channels + ch] = sample;
		}

		framesWritten++;
		s->cursor++;

		if (s->loopActive && s->cursor >= s->loopEnd) {
			s->cursor = s->loopStart + (s->cursor - s->loopEnd);
		}
		else if (!s->loopActive && s->cursor >= s->frameCount) {
			s->cursor = s->frameCount;
		}
	}

	if (pFramesRead) *pFramesRead = framesWritten;
	return MA_SUCCESS;
}

ma_result LoopingSample_seek(ma_data_source* pDataSource, ma_uint64 frameIndex) {
	((LoopingSample*)pDataSource)->cursor = frameIndex;
	return MA_SUCCESS;
}

ma_result LoopingSample_getCursor(ma_data_source* pDataSource, ma_uint64* pCursor) {
	*pCursor = ((LoopingSample*)pDataSource)->cursor;
	return MA_SUCCESS;
}

ma_result LoopingSample_getLength(ma_data_source* pDataSource, ma_uint64* pLength) {
	*pLength = ((LoopingSample*)pDataSource)->frameCount;
	return MA_SUCCESS;
}

ma_result LoopingSample_setLooping(ma_data_source* pDataSource, ma_bool32) {
	return MA_SUCCESS;
}

ma_result LoopingSample_getDataFormat(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel*, size_t) {
	LoopingSample* s = (LoopingSample*)pDataSource;
	
	*pFormat = ma_format_f32;
	*pChannels = s->channels;
	*pSampleRate = s->sampleRate;
	return MA_SUCCESS;
}

ma_data_source_vtable g_looping_vtable = {
	LoopingSample_read,
	LoopingSample_seek,
	LoopingSample_getDataFormat,
	LoopingSample_getCursor,
	LoopingSample_getLength,
	LoopingSample_setLooping,
	0
};

samplesModule::samplesModule(std::shared_ptr<voices> voiceManager, int maxPolyphony) : module(std::move(voiceManager)) {
	std::cout << "Samples module init" << std::endl;
	ma_result initResult = ma_engine_init(NULL, &this->engine);	

	if (initResult != MA_SUCCESS) {
		std::cout << "Engine init failed!" << std::endl;
		throw std::exception("Engine init failed!");
	}

	this->initEngine();
	this->maxPolyphony = maxPolyphony;
}

void samplesModule::initEngine() {
	std::cout << "Samples engine init" << std::endl;
	int loadedSamples = 0;
	const int predictedSamplesCount = this->voiceManager->getVoices().size() * NUMBER_OF_NOTES;

	for (auto& v : this->voiceManager->getVoices()) {
		for (int note = LOWEST_NOTE; note < (LOWEST_NOTE + NUMBER_OF_NOTES); note++) {
			std::vector<std::string> paths = v.getSamplesPath(note);

			std::string attackPath = paths[0];
			std::string sustainPath = paths[1];
			std::string releasePath = paths[2];

			sampleSet* set = new sampleSet();

			if (ma_sound_init_from_file(&engine, attackPath.c_str(), 0, NULL, NULL, &set->attack) != MA_SUCCESS) {
				std::cout << "Failed to load attack: " << attackPath << std::endl;
				continue;
			}

			ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 0, 0);
			ma_decoder decoder;

			if (ma_decoder_init_file(sustainPath.c_str(), &config, &decoder) != MA_SUCCESS) {
				std::cout << "Decoder init failed" << std::endl;
				continue;
			}

			ma_uint64 frameCount;
			ma_decoder_get_length_in_pcm_frames(&decoder, &frameCount);

			float* pcm = new float[frameCount * decoder.outputChannels];

			ma_decoder_read_pcm_frames(&decoder, pcm, frameCount, NULL);

			set->looping.data = pcm;
			set->looping.frameCount = frameCount;
			set->looping.channels = decoder.outputChannels;
			set->looping.sampleRate = decoder.outputSampleRate;
			set->looping.cursor = set->looping.loopStart;
			set->looping.loopStart = 1;
			set->looping.loopEnd = frameCount / 2;
			set->looping.fadeLength = frameCount / 32;
			set->looping.base.vtable = &g_looping_vtable;

			if (ma_sound_init_from_data_source(&engine, &set->looping, 0, NULL, &set->sustain) != MA_SUCCESS) {
				delete[] pcm;
				ma_decoder_uninit(&decoder);
				continue;
			}

			ma_decoder_uninit(&decoder);

			if (ma_sound_init_from_file(&engine, releasePath.c_str(), 0, NULL, NULL, &set->release) != MA_SUCCESS) {
				ma_sound_uninit(&set->attack);
				ma_sound_uninit(&set->sustain);

				std::cout << "Failed to load release: " << releasePath << std::endl;
				continue;
			}

			loadedSamples += 1;
			samples.emplace(std::pair{ v.getId(), note }, set);
		}
	}

	ma_engine_set_volume(&engine, 1.0f);

	std::cout << "Successfully loaded " << loadedSamples << " of " << predictedSamplesCount << " samples" << std::endl;
}

samplesModule::~samplesModule() {
	std::cout << "Destructor of Samples module";

	for (auto& [key, set] : samples) {
		ma_sound_uninit(&set->attack);
		ma_sound_uninit(&set->sustain);
		ma_sound_uninit(&set->release);

		delete[] set->looping.data;
		delete set;
	}

	ma_engine_uninit(&engine);

	std::cout << "Destructed";
}

void samplesModule::play(const noteSignal& signal, audioSignal& output) {
	int note = signal.note;

	if (note < LOWEST_NOTE || note >= (LOWEST_NOTE + NUMBER_OF_NOTES))
		return;
	
	for (const auto& v : this->voiceManager->getActiveVoices()) {
		int id = v.getId();
		auto it = samples.find({ id, note });

		if (it == samples.end())
			continue;

		if (signal.on) {
			it->second->startLoop();
		}			
		else {
			it->second->stopLoop();
		}			
	}
}

void sampleSet::startLoop() {
	this->looping.loopActive = true;
	this->looping.cursor = this->looping.loopStart;
	ma_sound_start(&sustain);
}

void sampleSet::stopLoop() {
	this->looping.loopActive = false;
	ma_sound_stop(&sustain);
}