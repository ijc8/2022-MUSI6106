
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

int TimeConvolution::getTailLength() const {
    // NOTE: The tail is the length of the impulse response minus one,
    // even if the user has provided fewer than `impulseResponse.size()` input samples.
    return impulseResponse.size() - 1;
}

FreqConvolution::FreqConvolution(const float *impulseResponse, int length, int blockLength)
: blockLength(blockLength), tailLength(length - 1 + blockLength), numBlocks((int)ceil((float)length / blockLength)),
  outputBlock(blockLength*2), saved(blockLength), inputBlockHistory(numBlocks*blockLength*2), impulseResponseBlocks(numBlocks) {
    CFft::createInstance(fft);
    fft->initInstance(blockLength*2, 1, CFft::kWindowHann, CFft::kNoWindow);
    for (int i = 0; i < numBlocks; i++) {
        int thisBlockLength = std::min(blockLength, length - i * blockLength);
        impulseResponseBlocks[i].resize(blockLength * 2);
        memcpy(&impulseResponseBlocks[i][0], &impulseResponse[i * blockLength], sizeof(float) * thisBlockLength);
        fft->doFft(impulseResponseBlocks[i].data(), impulseResponseBlocks[i].data());
    }
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

// Previous implementation of `multiplySpectra`, included for reference.
// This has the advantage of not needing information about the internal layout used by CFft;
// however, the constant spitRealImag/mergeRealImag calls adds considerable overhead.

// void FreqConvolution::multiplySpectra(float *spectrumC, const float *spectrumA, const float *spectrumB) {
//     std::vector<float> realA(blockLength+1), imagA(blockLength+1);
//     fft->splitRealImag(realA.data(), imagA.data(), spectrumA);

//     std::vector<float> realB(blockLength+1), imagB(blockLength+1);
//     fft->splitRealImag(realB.data(), imagB.data(), spectrumB);

//     float scale = blockLength * 2;
//     for (int i = 0; i < blockLength+1; i++) {
//         float tmp = realA[i];
//         realA[i] = (tmp * realB[i] - imagA[i] * imagB[i]) * scale;
//         imagA[i] = (tmp * imagB[i] + imagA[i] * realB[i]) * scale;
//     }
//     fft->mergeRealImag(spectrumC, realA.data(), imagA.data());
// }

static inline void addMultiplySpectra(float *output, const float *spectrumA, const float *spectrumB, int blockLength) {
    float scale = blockLength * 2;
    // Spectrum layout: re(0),re(1),re(2),...,re(size/2),im(size/2-1),...,im(1)
    output[0] += (spectrumA[0] * spectrumB[0]) * scale;
    output[blockLength] += (spectrumA[blockLength] * spectrumB[blockLength]) * scale;
    for (int i = 1; i < blockLength; i++) {
        float realA = spectrumA[i];
        float imagA = spectrumA[blockLength*2-i];
        float realB = spectrumB[i];
        float imagB = spectrumB[blockLength*2-i];
        output[i] += (realA * realB - imagA * imagB) * scale; // Real part
        output[blockLength*2-i] += (realA * imagB + imagA * realB) * scale; // Imaginary part
    }
}

void FreqConvolution::process(float *output, const float *input, int length) {
    for (int i = 0; i < length; i++) {
        output[i] = outputBlock[indexInBlock];
        inputBlockHistory[inputBlockIndex*blockLength*2 + indexInBlock++] = input[i];
        if (indexInBlock == blockLength) {
            // We have another full input block for processing.
            // Take the FFT in-place.
            float *inputBlock = &inputBlockHistory[inputBlockIndex*blockLength*2];
            std::fill(&inputBlock[blockLength], &inputBlock[blockLength*2], 0);
            // NOTE: Unfortunately CFft adds some overhead here from copying buffers around,
            // even though in this case we actually want the operation in-place.
            // This is the best we can do without using LaszloFft directly.
            fft->doFft(inputBlock, inputBlock);
            // Save time-domain residue from last round.
            memcpy(&saved[0], &outputBlock[blockLength], sizeof(float) * blockLength);
            // Sum results from circular convolutions (equivalent to linear convolutions due to zero-padding.)
            // This is a kind of "higher-level" convolution of blocks.
            std::fill(outputBlock.begin(), outputBlock.end(), 0);
            for (int j = 0; j < numBlocks; j++) {
                int idx = (inputBlockIndex - j + numBlocks) % numBlocks;
                addMultiplySpectra(outputBlock.data(), impulseResponseBlocks[j].data(), &inputBlockHistory[idx*blockLength*2], blockLength);
            }
            // Fourier Transform is linear, so we can just do the IFFT once after summing all the spectra.
            fft->doInvFft(outputBlock.data(), outputBlock.data());
            // Add back the residue from last time.
            CVectorFloat::add_I(outputBlock.data(), saved.data(), blockLength);
            inputBlockIndex = (inputBlockIndex + 1) % numBlocks;
            indexInBlock = 0;
        }
    }
}

int FreqConvolution::getTailLength() const {
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
    if (eCompMode == kTimeDomain) {
        conv = std::make_unique<TimeConvolution>(pfImpulseResponse, iLengthOfIr);
    } else if (eCompMode == kFreqDomain) {
        conv = std::make_unique<FreqConvolution>(pfImpulseResponse, iLengthOfIr, iBlockLength);
    } else {
        return Error_t::kFunctionInvalidArgsError;
    }
    return Error_t::kNoError;
}

Error_t CFastConv::reset() {
    conv.release();
    return Error_t::kNoError;
}

Error_t CFastConv::process(float *pfOutputBuffer, const float *pfInputBuffer, int iLengthOfBuffers) {
    if (!conv) return Error_t::kNotInitializedError;
    conv->process(pfOutputBuffer, pfInputBuffer, iLengthOfBuffers);
    return Error_t::kNoError;
}

int CFastConv::getTailLength() const {
    if (!conv) return -1;
    return conv->getTailLength();
}

Error_t CFastConv::flushBuffer(float* pfOutputBuffer) {
    if (!conv) return Error_t::kNotInitializedError;
    std::vector<float> zeros(getTailLength());
    return process(pfOutputBuffer, zeros.data(), zeros.size());
}
