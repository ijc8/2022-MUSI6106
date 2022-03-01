#include "Vibrato.h"
#include "LFO.h"

Vibrato::Vibrato(float sampleRate, float maxDepth, int iNumChannels = 1)
: m_fsampleRate(sampleRate), delayLine(ceil(sampleRate * maxDepth)), lfo(0), m_fdepth(0), m_iNumChannels(iNumChannels) {
}

void Vibrato::setFrequency(float frequency){
    lfo.setFrequency(frequency);
}

void Vibrato::setDepth(float depth){
    m_fdepth = depth;
}

void Vibrato::process(float **input, float **output, int numFrames) {

    float f_delayReadIdx;
    
    for (int c = 0; c < m_iNumChannels; c++){
        delayLine.reset();
        lfo.reset();
        for(int i = 0; i < numFrames; i++){
            delayLine.putPostInc(input[c][i]);
            f_delayReadIdx = lfo.process();
            output[c][i] = input[c][i] + delayLine.get(f_delayReadIdx);
        }
    }
    
}
