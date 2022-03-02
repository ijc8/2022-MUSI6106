#include "Vibrato.h"
#include "LFO.h"

Vibrato::Vibrato(float sampleRate, float maxDepth, int iNumChannels = 1)
: m_fsampleRate(sampleRate), delayLines(), lfo(0), m_fdepth(0), m_iNumChannels(iNumChannels) {
    assert(iNumChannels > 0);
    for (int i = 0; i < iNumChannels; i++) {
        int length = (int)ceilf(sampleRate * maxDepth * 2) + 1;
        delayLines.emplace_back(length);
    }
}

void Vibrato::setFrequency(float frequency) {
    lfo.setFrequency(frequency);
}

Error_t Vibrato::setDepth(float depth) {
    if (depth * m_fsampleRate > delayLines[0].getLength() / 2) {
        return Error_t::kFunctionInvalidArgsError;
    }
    lfo.setAmplitude(depth * m_fsampleRate);
    return Error_t::kNoError;
}

// TODO: getFrequency(), getDepth()

void Vibrato::process(float **input, float **output, int numFrames) {
    for (int c = 0; c < m_iNumChannels; c++) {
        RingBuffer<float> &delayLine = delayLines[c];
        for(int i = 0; i < numFrames; i++) {
            delayLine.putPostInc(input[c][i]);
            const float delayOffset = m_fdepth * lfo.process();
            output[c][i] = delayLine.get(delayLine.getWriteIdx() - delayLine.getLength() / 2 + delayOffset);
        }
    }
}
