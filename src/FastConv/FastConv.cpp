
#include <cassert>

#include "FastConv.h"

CFastConv::CFastConv() {
}

CFastConv::~CFastConv() {
    reset();
}

Error_t CFastConv::init(float *pfImpulseResponse, int iLengthOfIr, int iBlockLength /*= 8192*/, ConvCompMode_t eCompMode /*= kFreqDomain*/) {
    impulseResponse.assign(pfImpulseResponse, &pfImpulseResponse[iLengthOfIr]);
    history = std::make_unique<CRingBuffer<float>>(iLengthOfIr);
    mode = eCompMode;
    return Error_t::kNoError;
}

Error_t CFastConv::reset() {
    // TODO
    return Error_t::kNoError;
}

Error_t CFastConv::process(float *pfOutputBuffer, const float *pfInputBuffer, int iLengthOfBuffers) {
    if (mode == kTimeDomain) {
        processTimeDomain(pfOutputBuffer, pfInputBuffer, iLengthOfBuffers);
    } else if (mode == kFreqDomain) {
        processFreqDomain(pfOutputBuffer, pfInputBuffer, iLengthOfBuffers);
    } else {
        return Error_t::kFunctionIllegalCallError;
    }
    return Error_t::kNoError;
}

void CFastConv::processTimeDomain(float *output, const float *input, int length) {
    assert(false); // not implemented yet
    // plan:
    // - push input value on to history buffer
    // - take dot product of history buffer and impulse response (mind the flipped direction)
    // - write result to output
    // - repeat `length` times
}

void CFastConv::processFreqDomain(float *output, const float *input, int length) {
    assert(false); // not implemented yet
}

Error_t CFastConv::flushBuffer(float* pfOutputBuffer) {
    // TODO
    return Error_t::kNoError;
}
