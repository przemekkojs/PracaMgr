#pragma once

#include <string>
#include <vector>

struct noteSignal {
	unsigned char note;
	unsigned char channel;
	bool on;

	noteSignal(unsigned char note, unsigned char channel, bool on) : note(note), channel(channel), on(on) { }
	
	bool operator==(const noteSignal& other) const { return note == other.note && channel == other.channel && on == other.on; }
	std::string toString() const { return "note=" + std::to_string(note) + " channel=" + std::to_string(channel) + " state=" + (on ? "ON" : "OFF"); }
};

struct ccSignal {
	unsigned char cc;
	unsigned char channel;
	unsigned char value;

	ccSignal(unsigned char cc, unsigned char channel, unsigned char value) : cc(cc), channel(channel), value(value) { }

	bool operator==(const ccSignal& other) const { return cc == other.cc && channel == other.channel && value == other.value; }
	std::string toString() const { return "cc=" + std::to_string(cc) + " channel=" + std::to_string(channel) + " value=" + std::to_string(value); }
};

struct audioSignal {
	float left = 0.0f;
	float right = 0.0f;

	std::vector<float> buffer;

	void clear() {
		left = 0.0f;
		right = 0.0f;
	}
};

const audioSignal EMPTY_AUDIO_SIGNAL = audioSignal();
const noteSignal EMPTY_NOTE_SIGNAL = noteSignal(128, 0, false);