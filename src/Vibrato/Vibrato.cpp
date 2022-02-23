#include "Vibrato.h"

Vibrato::Vibrato(float sampleRate, float maxDepth)
: sampleRate(sampleRate), delayLine(ceil(sampleRate * maxDepth)), lfo(0), depth(0) {
}

void Vibrato::process(float **input, float **output, int numFrames) {

}
