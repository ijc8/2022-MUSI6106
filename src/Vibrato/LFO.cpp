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

void LFO::setFrequency(float frequency) {
    this->frequency = frequency;
}

float LFO::process() {
    float result = wavetable.get(index);
    index += sampleRate / frequency * wavetable.getLength();
    return result;
}
