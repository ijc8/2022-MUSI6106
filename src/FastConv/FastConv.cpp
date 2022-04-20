
#include <cassert>
#include <iostream>

#include "FastConv.h"
#include "Fft.h"
#include "Vector.h"

TimeConvolution::TimeConvolution(const float *impulseResponse, int length)
: impulseResponse(impulseResponse, &impulseResponse[length]), history(length) {
}

void TimeConvolution::process(float *output, const float *input, int length) {
    for (int i = 0; i < length; i++) {
        history.putPostInc(input[i]);
        float acc = 0;
        for (int j = 0; j < impulseResponse.size(); j++) {
            acc += impulseResponse[j] * history.get(-j);
        }
        output[i] = acc;
        history.getPostInc();
    }
}

int TimeConvolution::getTailLength() {
    // NOTE: The tail is the length of the impulse response minus one,
    // even if the user has provided fewer than `impulseResponse.size()` input samples.
    return impulseResponse.size() - 1;
}

FreqConvolution::FreqConvolution(const float *impulseResponse, int length, int blockLength)
: blockLength(blockLength), tailLength(length - 1 + blockLength) {
    CFft::createInstance(fft);
    fft->initInstance(blockLength*2, 1, CFft::kWindowHann, CFft::kNoWindow);
    inputBuffer = std::make_unique<CRingBuffer<float>>(blockLength + 1);
    outputBuffer = std::make_unique<CRingBuffer<float>>(blockLength + 1);
    for (int i = 0; i < blockLength; i++) {
        outputBuffer->putPostInc(0);
    }
    int numBlocks = (int)ceil((float)length / blockLength);
    inputBlockHistory = std::make_unique<CRingBuffer<std::vector<float>>>(numBlocks);
    std::vector<float> tmp(blockLength*2);
    impulseResponseBlocks.resize(numBlocks);
    for (int i = 0; i < numBlocks; i++) {
        inputBlockHistory->putPostInc(tmp);
        int thisBlockLength = std::min(blockLength, length - i * blockLength);
        impulseResponseBlocks[i].resize(blockLength * 2);
        memcpy(&impulseResponseBlocks[i][0], &impulseResponse[i * blockLength], sizeof(float) * thisBlockLength);
        fft->doFft(impulseResponseBlocks[i].data(), impulseResponseBlocks[i].data());
    }
    saved.resize(blockLength);
}

FreqConvolution::~FreqConvolution() {
    CFft::destroyInstance(fft);
}

// Time-domain implementation of circular convolution, for reference/testing:
// void circularConvolve(float *output, const float *a, const float *b, int length) {
//     for (int i = 0; i < length; i++) {
//         float acc = 0;
//         for (int j = 0; j < length; j++) {
//             acc += a[j] * b[(i - j + length) % length];
//         }
//         output[i] = acc;
//     }
// }

void FreqConvolution::multiplySpectra(float *spectrumC, const float *spectrumA, const float *spectrumB) {
    float realA[blockLength+1], imagA[blockLength+1];
    fft->splitRealImag(realA, imagA, spectrumA);

    float realB[blockLength+1], imagB[blockLength+1];
    fft->splitRealImag(realB, imagB, spectrumB);

    float scale = blockLength * 2;
    float realC[blockLength+1], imagC[blockLength+1];
    for (int i = 0; i < blockLength+1; i++) {
        realC[i] = (realA[i] * realB[i] - imagA[i] * imagB[i]) * scale;
        imagC[i] = (realA[i] * imagB[i] + imagA[i] * realB[i]) * scale;
    }
    fft->mergeRealImag(spectrumC, realC, imagC);
}

void FreqConvolution::process(float *output, const float *input, int length) {
    float convolution[blockLength*2];
    for (int i = 0; i < length; i++) {
        inputBuffer->putPostInc(input[i]);
        if (inputBuffer->getNumValuesInBuffer() >= blockLength) {
            std::vector<float> inputBlock(blockLength*2);
            inputBuffer->getPostInc(inputBlock.data(), blockLength);
            fft->doFft(inputBlock.data(), inputBlock.data());
            inputBlockHistory->putPostInc(inputBlock);
            int numBlocks = impulseResponseBlocks.size();
            // Sum results from circular convolutions (equivalent to linear convolutions due to zero-padding.)
            float acc[blockLength*2] = {0};
            for (int j = 0; j < numBlocks; j++) {
                multiplySpectra(convolution, impulseResponseBlocks[j].data(), inputBlockHistory->get(-j).data());
                CVectorFloat::add_I(acc, convolution, blockLength*2);
            }
            // Fourier Transform is linear, so we can just do the IFFT once after summing all the spectra.
            fft->doInvFft(acc, acc);
            // Output the first half.
            for (int k = 0; k < blockLength; k++) {
                outputBuffer->putPostInc(acc[k] + saved[k]);
            }
            // Save the second half.
            memcpy(&saved[0], &acc[blockLength], sizeof(float) * blockLength);
            inputBlockHistory->getPostInc();
        }
        output[i] = outputBuffer->getPostInc();
    }
}

int FreqConvolution::getTailLength() {
    // For frequency domain convolution, the tail also includes
    // the block size due to latency (initial output of zeros).
    return tailLength;
}

CFastConv::CFastConv() {
}

CFastConv::~CFastConv() {
    reset();
}

Error_t CFastConv::init(float *pfImpulseResponse, int iLengthOfIr, int iBlockLength /*= 8192*/, ConvCompMode_t eCompMode /*= kFreqDomain*/) {
    irLength = iLengthOfIr;
    mode = eCompMode;
    if (mode == kTimeDomain) {
        timeConv = std::make_unique<TimeConvolution>(pfImpulseResponse, iLengthOfIr);
    } else {
        freqConv = std::make_unique<FreqConvolution>(pfImpulseResponse, iLengthOfIr, iBlockLength);
    }
    return Error_t::kNoError;
}

Error_t CFastConv::reset() {
    timeConv.release();
    freqConv.release();
    return Error_t::kNoError;
}

Error_t CFastConv::process(float *pfOutputBuffer, const float *pfInputBuffer, int iLengthOfBuffers) {
    if (mode == kTimeDomain) {
        timeConv->process(pfOutputBuffer, pfInputBuffer, iLengthOfBuffers);
    } else if (mode == kFreqDomain) {
        freqConv->process(pfOutputBuffer, pfInputBuffer, iLengthOfBuffers);
    } else {
        return Error_t::kFunctionIllegalCallError;
    }
    return Error_t::kNoError;
}

int CFastConv::getTailLength() {
    if (mode == kTimeDomain) {
        return timeConv->getTailLength();
    } else if (mode == kFreqDomain) {
        return freqConv->getTailLength();
    }
    return -1;
}

Error_t CFastConv::flushBuffer(float* pfOutputBuffer) {
    int length = getTailLength();
    float zeros[length] = {0};
    process(pfOutputBuffer, zeros, length);
    return Error_t::kNoError;
}
