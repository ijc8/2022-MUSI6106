#include <iostream>

#include "CombFilter.h"

// The `+ 1` here ensures that the user can use the max delay
// without any ambiguity from the ring buffer when `head == tail`.
CCombFilterBase::CCombFilterBase(float maxDelay, float sampleRate)
: sampleRate(sampleRate), delayline(ceil(maxDelay * sampleRate) + 1) {
    setDelay(maxDelay);
}

Error_t CCombFilterBase::setGain(float _gain) {
    gain = _gain;
    return Error_t::kNoError;
}

float CCombFilterBase::getGain() {
    return gain;
}

Error_t CCombFilterBase::setDelay(float delay) {
    int delayInSamples = round(delay * sampleRate);
    if (delay < 0 || delayInSamples > delayline.getLength() - 1) {
        // Delay is negative or greater than user-specified `maxDelay`.
        return Error_t::kFunctionInvalidArgsError;
    }
    delayline.setReadIdx(delayline.getWriteIdx() - delayInSamples);
    return Error_t::kNoError;
}

float CCombFilterBase::getDelay() {
    return delayline.getNumValuesInBuffer() / sampleRate;
}


Error_t FIRFilter::process(float *input, float *output, int numFrames) {
    if (numFrames < 0) {
        return Error_t::kFunctionInvalidArgsError;
    }
    for (int i = 0; i < numFrames; i++) {
        float x = input[i];
        // NOTE: We push first to ensure correct handling of zero-delay case.
        delayline.push(x);
        output[i] = x + gain * delayline.pop();
    }
    return Error_t::kNoError;
}


Error_t IIRFilter::setDelay(float delay) {
    if (round(delay * sampleRate) < 1) {
        // Can't have zero-delay feedback loop.
        return Error_t::kFunctionInvalidArgsError;
    }
    return CCombFilterBase::setDelay(delay);
}

Error_t IIRFilter::process(float *input, float *output, int numFrames) {
    if (numFrames < 0) {
        return Error_t::kFunctionInvalidArgsError;
    }
    for (int i = 0; i < numFrames; i++) {
        float y = input[i] + gain * delayline.pop();
        output[i] = y;
        delayline.push(y);
    }
    return Error_t::kNoError;
}
