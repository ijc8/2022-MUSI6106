#include "Vibrato.h"

Vibrato::Vibrato(float sampleRate, float maxDepth)
: sampleRate(sampleRate), delayLine(ceil(sampleRate * maxDepth)), lfo(0), depth(0) {
}

void Vibrato::process(float **input, float **output, int numFrames) {
    //Write the current sample to the delay line
    //Read the delay sample
    //Add delayed sample to current sample
    //Increment both wavetable and modulated delay line appropriately
        //Using LFO's output to modify the delay
    int numChannels;
    for (int c = 0; c < numChannels; c++){
        for(int i = 0; i < numFrames; i++){
            
        }
    }

}
