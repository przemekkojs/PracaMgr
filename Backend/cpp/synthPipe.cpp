#include "../h/synthPipe.h"

synthPipe::synthPipe() : params(), adsr() {
    rng.seed(42);
}

void synthPipe::load(synthPipeParams& params) {
    this->params = params;

    this->adsr.setAttack(0.04f);
    this->adsr.setDecay(0.05f);
    this->adsr.setSustain(0.7f);
    this->adsr.setRelease(0.1f);

    delayLine.assign(static_cast<std::vector<float, std::allocator<float>>::size_type>((int)params.delaySamples) + 4, 0.0f);
    jetDelayLine.assign(static_cast<std::vector<float, std::allocator<float>>::size_type>((int)params.jetDelaySamples) + 4, 0.0f);

    writeIdx = 0;
    jetIdx = 0;
}

float synthPipe::pinkNoise() {
    static float b0 = 0, b1 = 0, b2 = 0;

    float white = noise(rng);

    b0 = 0.99886f * b0 + white * 0.0555179f;
    b1 = 0.99332f * b1 + white * 0.0750759f;
    b2 = 0.96900f * b2 + white * 0.1538520f;

    return (b0 + b1 + b2) * 0.33f;
}

float synthPipe::brownNoise() {
    static float y = 0.0f;
    float white = noise(rng);

    y += white * 0.02f;
    y = std::clamp(y, -1.0f, 1.0f);

    return y * 3.5f;
}

float synthPipe::whiteNoise() {
    return noise(rng);
}

void synthPipe::noteOn() {
    this->adsr.noteOn();

    for (int i = 0; i < delayLine.size(); i++) {
        delayLine[i] = 0.01f * fastNoise();
    }

    lossState = 0;
}

void synthPipe::noteOff() {
    this->adsr.noteOff();
}

float synthPipe::fastNoise() {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return (state * 2.3283064365387e-10f) * 2.0f - 1.0f;
}

float synthPipe::lossFilter(float x) {
    float a = params.baseParams.lossFilterCoeff;
    float y = x - loss_x1 + a * loss_y1;

    loss_x1 = x;
    loss_y1 = y;

    return y;
}

float synthPipe::lowpass(float x) {
    loopLP = (1 - params.baseParams.lowpassCoeff) * x + params.baseParams.lowpassCoeff * loopLP;
    return loopLP;
}

float synthPipe::jetLowpass(float x) {
    jetLP = (1 - params.baseParams.jetLowpassCoeff) * x + params.baseParams.jetLowpassCoeff * jetLP;
    return jetLP;
}

float synthPipe::nonlinear(float x, float env) {
    float offset = 0.05f * (1 + env);
    float input = std::clamp(x + offset, -1.0f, 1.0f);
    return input - (input * input * input);
}

float synthPipe::allpass(float x) {
    float a = 0.1f;

    float y = -a * x + ap_x1 + a * ap_y1;
    ap_x1 = x;
    ap_y1 = y;

    return y;
}

bool synthPipe::isActive() {
    return adsr.isActive() && !delayLine.empty() && !jetDelayLine.empty();
}

float synthPipe::processEnvelope() {
    return adsr.process();
}

float synthPipe::readPipe() {
    float tap = (float)writeIdx - params.delaySamples;

    if (tap < 0)
        tap += (float)delayLine.size();

    int i1 = (int)tap;
    int i2 = (i1 + 1) % delayLine.size();
    float f = tap - (float)i1;

    return delayLine[i1] + f * (delayLine[i2] - delayLine[i1]);
}

float synthPipe::processPipeFilter(float pipeOut) {
    float filtered = pipeOut - lastSample;
    lastSample = pipeOut * 0.995f;
    return filtered;
}

float synthPipe::computeBreath(float env, float pipeOut) {
    float noiseAmount = params.baseParams.noiseGain * env;

    if (env > 0.9f)
        noiseAmount *= 1.5f;

    float turbulence = whiteNoise() * noiseAmount;
    float driftNoise = fastNoise() * 0.002f;

    smoothedNoise = 0.05f * turbulence + 0.95f * smoothedNoise;
    windDrift = 0.9999f * windDrift + driftNoise;

    return (params.baseParams.excitationGain * env) + smoothedNoise + windDrift;
}

float synthPipe::processJet(float breath, float pipeOut) {
    float pressureDiff = breath - (pipeOut * params.baseParams.reflection);

    jetDelayLine[jetIdx] = pressureDiff;
    int jetReadIdx = (jetIdx + 1) % jetDelayLine.size();

    float jetDelayed = jetDelayLine[jetReadIdx];
    jetIdx = jetReadIdx;

    return jetDelayed;
}

float synthPipe::processExcitation(float jet, float env) {
    float x = std::clamp(jet, -1.0f, 1.0f);
    return nonlinear(x, env);
}

float synthPipe::processFeedback(float excitation, float pipeOut) {
    float input = excitation + (pipeOut * params.baseParams.loopFeedbackGain);
    return lowpass(input);
}

void synthPipe::writePipe(float input) {
    delayLine[writeIdx] = input;
    writeIdx = (writeIdx + 1) % delayLine.size();
}

float synthPipe::processOutput(float pipeOut) {
    float dcBlocker = pipeOut - lastPipeOut;
    lastPipeOut = pipeOut * 0.995f;
    return dcBlocker * 0.3f;
}

float synthPipe::process() {
    if (!isActive())
        return 0.0f;

    float env = processEnvelope();
    float pipeOut = readPipe();
    float filteredOut = processPipeFilter(pipeOut);

    float breath = computeBreath(env, pipeOut);
    float jet = processJet(breath, pipeOut);
    float excitation = processExcitation(jet, env);
    float pipeInput = processFeedback(excitation, pipeOut);

    writePipe(pipeInput);

    return processOutput(pipeOut);
}


float reedPipe::computeBreath(float env, float pipeOut) {
    float pm = (params.baseParams.breathBase + params.baseParams.breathEnvAmount * env);
    float noise = fastNoise() * params.baseParams.breathNoiseGain;

    return pm + noise;
}

float reedPipe::processJet(float breath, float pipeOut) {
    float deltaP = breath - pipeOut;

    jetDelayLine[jetIdx] = deltaP;

    int readIdx = (jetIdx + (int)params.baseParams.jetDelayOffset) % jetDelayLine.size();
    float delayed = jetDelayLine[readIdx];

    jetIdx = (jetIdx + 1) % jetDelayLine.size();

    return delayed;
}

float reedPipe::processExcitation(float deltaP, float env) {
    float pm = deltaP;
    float buzz = fastNoise() * params.baseParams.buzzGain * env;

    float opening = params.baseParams.reedOpening - params.baseParams.reedBias * pm + buzz;
    if (opening < 0.0f)
        opening = 0.0f;

    opening = params.baseParams.reedHysteresis * lastOpening + (1.0f - params.baseParams.reedHysteresis) * opening;
    lastOpening = opening;

    float flow = opening * std::sqrt(std::fabs(pm) + 1e-6f);
    flow *= (pm >= 0.0f ? 1.0f : -1.0f);
    flow += params.baseParams.reedLeak * pm;

    float drive = params.baseParams.reedNonlin * (1.0f + params.baseParams.reedNonlinEnv * env);
    float out = std::tanh(flow * drive);

    float f = params.baseParams.resonanceFreq / SAMPLE_RATE;
    float hp = out - resonance_z1;
    float bp = resonance_z1 - resonance_z2;

    resonance_z1 += f * hp;
    resonance_z2 += f * bp;

    out += params.baseParams.resonanceGain * bp;

    if (env < params.baseParams.attackEnvThreshold)
        out += fastNoise() * params.baseParams.attackNoiseGain;

    return out * params.baseParams.reedGain;
}

float reedPipe::processFeedback(float flow, float pipeOut) {
    float input = flow + (pipeOut * params.baseParams.loopFeedbackGain);

    input = std::tanh(input * params.baseParams.feedbackNonlin);

    return lowpass(input);
}



float flutePipeModel::nonlinear(float x, float env) {
    float drive = 1.0f + env * 3.0f;
    float y = x * drive;

    return std::tanh(y);
}

float flutePipeModel::processExcitation(float jet, float env) {
    float x = jetLowpass(jet);

    x = std::clamp(x, -1.0f, 1.0f);
    return nonlinear(x, env);
}



float stringPipeModel::nonlinear(float x, float env) {
    float drive = 1.0f + env * 3.0f;
    float y = x * drive;

    return std::tanh(y);
}

float stringPipeModel::processExcitation(float jet, float env) {
    float x = jetLowpass(jet);

    x = std::clamp(x, -1.0f, 1.0f);
    return nonlinear(x, env);
}



float principalPipeModel::nonlinear(float x, float env) {
    float drive = 1.0f + env * 3.0f;
    float y = x * drive;

    return std::tanh(y);
}

float principalPipeModel::processExcitation(float jet, float env) {
    float x = jetLowpass(jet);

    x = std::clamp(x, -1.0f, 1.0f);
    return nonlinear(x, env);
}



float reedPipeModel::nonlinear(float x, float env) {
    float drive = 1.0f + env * 3.0f;
    float y = x * drive;

    return std::tanh(y);
}