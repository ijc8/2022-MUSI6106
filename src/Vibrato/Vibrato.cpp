#include "Vibrato.h"
#include "LFO.h"

Vibrato::Vibrato(float sampleRate, float maxDepth, int numChannels = 1)
: sampleRate(sampleRate), lfo(sampleRate) {
    assert(numChannels > 0);
    for (int i = 0; i < numChannels; i++) {
        int length = (int)ceilf(sampleRate * maxDepth * 2) + 1;
        delayLines.emplace_back(new RingBuffer<float>(length));
    }
}

void Vibrato::setFrequency(float frequency) {
    lfo.setFrequency(frequency);
}

Error_t Vibrato::setDepth(float depth) {
    if (depth * sampleRate > delayLines[0]->getLength() / 2) {
        return Error_t::kFunctionInvalidArgsError;
    }
    lfo.setAmplitude(depth * sampleRate);
    return Error_t::kNoError;
}

// TODO: getFrequency(), getDepth()

void Vibrato::process(float **input, float **output, int numFrames) {
    const float baseDelay = delayLines[0]->getLength() / 2;
    for (int c = 0; c < delayLines.size(); c++) {
        RingBuffer<float> &delayLine = *delayLines[c];
        for(int i = 0; i < numFrames; i++) {
            delayLine.putPostInc(input[c][i]);
            const float tap = delayLine.getWriteIdx() - (baseDelay + lfo.process());
            output[c][i] = delayLine.get(tap);
        }
    }
}
