
#include <cassert>
#include <iostream>

#include "FastConv.h"
#include "Fft.h"
#include "Vector.h"

CFastConv::CFastConv() {
}

CFastConv::~CFastConv() {
    reset();
}

Error_t CFastConv::init(float *pfImpulseResponse, int iLengthOfIr, int iBlockLength /*= 8192*/, ConvCompMode_t eCompMode /*= kFreqDomain*/) {
    impulseResponse.assign(pfImpulseResponse, &pfImpulseResponse[iLengthOfIr]);
    blockLength = iBlockLength;
    mode = eCompMode;
    if (mode == kTimeDomain) {
        history = std::make_unique<CRingBuffer<float>>(iLengthOfIr);
    } else {
        inputBuffer = std::make_unique<CRingBuffer<float>>(blockLength + 1);
        outputBuffer = std::make_unique<CRingBuffer<float>>(blockLength + 1);
        for (int i = 0; i < blockLength; i++) {
            outputBuffer->putPostInc(0);
        }
        int numBlocks = (int)ceil((float)iLengthOfIr / iBlockLength);
        inputBlockHistory = std::make_unique<CRingBuffer<std::vector<float>>>(numBlocks);
        std::vector<float> tmp(blockLength*2);
        impulseResponseBlocks.resize(numBlocks);
        for (int i = 0; i < numBlocks; i++) {
            inputBlockHistory->putPostInc(tmp);
            int thisBlockLength = std::min(blockLength, iLengthOfIr - i * blockLength);
            impulseResponseBlocks[i].resize(blockLength * 2);
            memcpy(&impulseResponseBlocks[i][0], &pfImpulseResponse[i * blockLength], sizeof(float) * thisBlockLength);
        }
        saved.resize(blockLength*2);
    }
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
    for (int i = 0; i < length; i++) {
        history->putPostInc(input[i]);
        float acc = 0;
        for (int j = 0; j < impulseResponse.size(); j++) {
            acc += impulseResponse[j] * history->get(-j);
        }
        output[i] = acc;
        history->getPostInc();
    }
}

// TODO: Do this via FFT (see comments below).
void circularConvolve(float *output, const float *a, const float *b, int length) {
    for (int i = 0; i < length; i++) {
        float acc = 0;
        for (int j = 0; j < length; j++) {
            acc += a[j] * b[(i - j + length) % length];
        }
        output[i] = acc;
    }
}

void CFastConv::processFreqDomain(float *output, const float *input, int length) {
    float convolution[blockLength*2];
    for (int i = 0; i < length; i++) {
        inputBuffer->putPostInc(input[i]);
        if (inputBuffer->getNumValuesInBuffer() >= blockLength) {
            std::vector<float> inputBlock(blockLength*2);
            inputBuffer->getPostInc(inputBlock.data(), blockLength);
            inputBlockHistory->putPostInc(inputBlock);
            int wtf = impulseResponseBlocks.size();
            float acc[blockLength] = {0};
            for (int j = 0; j < wtf; j++) {
                circularConvolve(convolution, impulseResponseBlocks[j].data(), inputBlockHistory->get(-j).data(), blockLength*2);
                // Add the results of the circular convolution. (Due to zero-padding, should be equivalent to linear convolution.)
                CVectorFloat::add_I(saved.data(), convolution, blockLength*2);
            }
            // Output the first half.
            for (int k = 0; k < blockLength; k++) {
                outputBuffer->putPostInc(saved[k]);
            }
            // Save the second half.
            memcpy(&saved[0], &saved[blockLength], sizeof(float) * blockLength);
            memset(&saved[blockLength], 0, sizeof(float) * blockLength);
            inputBlockHistory->getPostInc();
        }
        output[i] = outputBuffer->getPostInc();
    }
}

// void CFastConv::processFreqDomain(float *output, const float *input, int length) {
//     int IdxWrite = 0;
//     float* pfInputBlockBuffer = new float[2 * blockLength]{ 0 };
//     int NumofBlock = static_cast<int>(std::ceil(impulseResponse.size()) / static_cast<float>(blockLength));
//     int BlockIdxRead = NumofBlock - 1;
//     float** ppfOutputBlockBuffer = new float* [NumofBlock];
//     float* pfReal_BlockFFTCurrent = new float[blockLength + 1];
//     float* pfImage_BlockFFTCurrent = new float[blockLength + 1];
//     CFft* pforFFT = 0;
//     CFft::createInstance(pforFFT);
//     pforFFT->initInstance(blockLength);
//     CFft::complex_t* pfComplexNum = new float[2 * blockLength];
//     float* pfReal_FFT = new float[blockLength + 1];
//     float* pfImage_FFT = new float[blockLength + 1];
//     float* pf_IFFT = new float[2 * blockLength]{ 0 };
//     int IdxBlock_Write = 0;
//     float** ppfReal_IRFreq = new float* [NumofBlock];
//     float** ppfImage_IRFreq = new float* [NumofBlock];
//     for (int i = 0; i < NumofBlock; i++) {
//         ppfReal_IRFreq[i] = new float[blockLength + 1]{ 0 };
//         ppfImage_IRFreq[i] = new float[blockLength + 1]{ 0 };
//         ppfOutputBlockBuffer[i] = new float[blockLength] {0};
//         for (int j = 0; j < blockLength; j++) {
//             if (i * blockLength + j < impulseResponse.size())
//                 pf_IFFT[j] = impulseResponse[i * blockLength + j];
//             else
//                 pf_IFFT[j] = 0;
//         }

//         for (int j = blockLength; j < 2 * blockLength; j++)
//             pf_IFFT[j] = 0;

//         pforFFT->doFft(pfComplexNum, pf_IFFT);
//         pforFFT->splitRealImag(ppfReal_IRFreq[i], ppfImage_IRFreq[i], pfComplexNum);
//     }

//     for (int i = 0; i < length; i++) {
//         pfInputBlockBuffer[IdxWrite + blockLength] = input[i];

//         output[i] = ppfOutputBlockBuffer[BlockIdxRead][IdxWrite];

//         IdxWrite++;

//         if (IdxWrite == blockLength) {
//             IdxWrite = 0;
//             for (int j = 0; j < blockLength; j++) {
//                 ppfOutputBlockBuffer[BlockIdxRead][j] = 0;
//             }
//             pforFFT->doFft(pfComplexNum, pfInputBlockBuffer);
//             pforFFT->splitRealImag(pfReal_BlockFFTCurrent, pfImage_BlockFFTCurrent, pfComplexNum);
//             for (int j = 0; j < NumofBlock; j++) {
//                 for (int i = 0; i <= blockLength; i++) {
//                     pfReal_FFT[i] = (pfReal_BlockFFTCurrent[i] * ppfReal_IRFreq[j][i] - pfImage_BlockFFTCurrent[i] * ppfImage_IRFreq[j][i]) * 2 * blockLength;
//                     pfImage_FFT[i] = (pfReal_BlockFFTCurrent[i] * ppfImage_IRFreq[j][i] + pfImage_BlockFFTCurrent[i] * ppfReal_IRFreq[j][i]) * 2 * blockLength;
//                 }
//                 pforFFT->mergeRealImag(pfComplexNum, pfReal_FFT, pfImage_FFT);
//                 pforFFT->doInvFft(pf_IFFT, pfComplexNum);
//                 const int l_iWriteBlockNum = (IdxBlock_Write + j) % NumofBlock;
//                 for (int k = 0; k < blockLength; k++) {
//                     ppfOutputBlockBuffer[l_iWriteBlockNum][k] += pf_IFFT[k + blockLength];
//                 }
//             }
//             for (int j = 0; j < blockLength; j++) {
//                 pfInputBlockBuffer[j] = pfInputBlockBuffer[j + blockLength];
//             }

//             BlockIdxRead = IdxBlock_Write;
//             IdxBlock_Write = (IdxBlock_Write + 1) % NumofBlock;
//         }
//     }

//     for (int i = 0; i < NumofBlock; i++) {
//         delete[] ppfOutputBlockBuffer[i];
//         delete[] ppfReal_IRFreq[i];
//         delete[] ppfImage_IRFreq[i];
//     }
//     delete[] ppfReal_IRFreq;
//     delete[] ppfImage_IRFreq;
//     delete[] pfInputBlockBuffer;
//     delete[] ppfOutputBlockBuffer;

//     ppfReal_IRFreq = 0;
//     ppfImage_IRFreq = 0;
//     pfInputBlockBuffer = 0;
//     ppfOutputBlockBuffer = 0;

//     delete[] pf_IFFT;
//     delete[] pfReal_FFT;
//     delete[] pfImage_FFT;
//     delete[] pfReal_BlockFFTCurrent;
//     delete[] pfImage_BlockFFTCurrent;
//     pf_IFFT = 0;
//     pfReal_FFT = 0;
//     pfImage_FFT = 0;


//     delete[] pfComplexNum;
//     pfComplexNum = 0;

//     CFft::destroyInstance(pforFFT);
//     pforFFT = 0;
// }

Error_t CFastConv::flushBuffer(float* pfOutputBuffer) {
    // NOTE: The tail is the length of the impulse response minus one,
    // even if the user has provided fewer than `impulseResponse.size()` input samples.
    int length = impulseResponse.size() - 1;
    if (mode == kFreqDomain) {
        // For frequency domain convolution, the tail also includes
        // the block size due to latency (initial output of zeros).
        length += blockLength;
    }
    float zeros[length] = {0};
    process(pfOutputBuffer, zeros, length);
    return Error_t::kNoError;
}
