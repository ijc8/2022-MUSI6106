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

float Vibrato::getFrequency() const {
    return lfo.getFrequency();
}

float Vibrato::getDepth() const {
    return lfo.getAmplitude() / sampleRate;
}

void Vibrato::process(float **input, float **output, int numFrames) {
    for (int c = 0; c < delayLines.size(); c++) {
        RingBuffer<float> &delayLine = *delayLines[c];
        for(int i = 0; i < numFrames; i++) {
            delayLine.putPostInc(input[c][i]);
            const float baseDelay = delayLine.getWriteIdx() - delayLine.getLength() / 2;
            output[c][i] = delayLine.get(baseDelay + lfo.process());
        }
    }
}
