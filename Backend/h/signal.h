#pragma once

struct noteSignal {
	unsigned char note;
	unsigned char channel;
	bool on;

	noteSignal(unsigned char note, unsigned char channel, bool on)
		: note(note), channel(channel), on(on) { }

	bool operator==(const noteSignal& other) const {
		return note == other.note && channel == other.channel && on == other.on;
	}
};

struct ccSignal {
	unsigned char cc;
	unsigned char channel;
	unsigned char value;

	ccSignal(unsigned char cc, unsigned char channel, unsigned char value)
		: cc(cc), channel(channel), value(value) { }

	bool operator==(const ccSignal& other) const {
		return cc == other.cc && channel == other.channel && value == other.value;
	}
};

struct audioSignal {

};