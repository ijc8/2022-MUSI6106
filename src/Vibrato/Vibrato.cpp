#include <cassert>

#include "Vibrato.h"
#include "LFO.h"

Vibrato::Vibrato(float sampleRate, float maxDepth, int numChannels = 1)
: sampleRate(sampleRate), lfo(sampleRate), maxDepth(maxDepth) {
    assert(numChannels > 0);
    for (int i = 0; i < numChannels; i++) {
        int length = (int)roundf(sampleRate * maxDepth * 2) + 2;
        delayLines.emplace_back(new RingBuffer<float>(length));
    }
}

void Vibrato::setFrequency(float frequency) {
    lfo.setFrequency(frequency);
}

Error_t Vibrato::setDepth(float depth) {
    if (depth > maxDepth) {
        return Error_t::kFunctionInvalidArgsError;
    }
    lfo.setAmplitude(depth * sampleRate);
    return Error_t::kNoError;
}

float Vibrato::getFrequency() const {
    return lfo.getFrequency();
}

float Vibrato::getDepth() const {
    return lfo.getAmplitude() / sampleRate;
}

void Vibrato::process(float **input, float **output, int numFrames) {
    const float baseDelay = maxDepth * sampleRate + 1;
    for (int c = 0; c < delayLines.size(); c++) {
        RingBuffer<float> &delayLine = *delayLines[c];
        for(int i = 0; i < numFrames; i++) {
            delayLine.putPostInc(input[c][i]);
            const float offset = baseDelay + lfo.process();
            assert(offset > 0 && offset <= delayLine.getLength() - 1);
            const float tap = delayLine.getWriteIdx() - offset;
            output[c][i] = delayLine.get(tap);
        }
    }
}
