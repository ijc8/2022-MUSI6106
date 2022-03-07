#include <math.h>

#include "LFO.h"

LFO::LFO(float sampleRate, int resolution) : sampleRate(sampleRate), wavetable(resolution) {
    for (int i = 0; i < resolution; i++) {
        wavetable.putPostInc(sinf(2 * M_PI * i / resolution));
    }
}

float LFO::getFrequency() const {
    return frequency;
}

float LFO::getAmplitude() const {
    return amplitude;
}

void LFO::setAmplitude(float amplitude) {
    this->amplitude = amplitude;
}

void LFO::setFrequency(float frequency) {
    this->frequency = frequency;
}

void LFO::reset(){
    this->wavetable.reset();
}

float LFO::process() {
    float result = wavetable.get(index);
    index += frequency / sampleRate * wavetable.getLength();
    return result * amplitude;
}
