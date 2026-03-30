#pragma once

#include "signal.h"

class metric {
public:
	metric();
	~metric();

	virtual float value(audioSignal& refSignal, audioSignal& compSignal) = 0;

protected:
	
};

class metric1 : public metric {
public:
	metric1();
	~metric1();

	float value(audioSignal& refSignal, audioSignal& compSignal) override;

private:

};

class metric2 : public metric {
public:
	metric2();
	~metric2();

	float value(audioSignal& refSignal, audioSignal& compSignal) override;

private:

};