
#include <cassert>

#include "FastConv.h"
#include "Fft.h"

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

void CFastConv::processFreqDomain(float *output, const float *input, int length) {
    int BlockLength = 32;
    int IdxWrite = 0;
    float* pfInputBlockBuffer = new float[2 * BlockLength]{ 0 };
    int NumofBlock = static_cast<int>(std::ceil(static_cast<float>(impulseResponse.size()) / static_cast<float>(BlockLength)));
    int BlockIdxRead = NumofBlock - 1;
    float** ppfOutputBlockBuffer = new float* [NumofBlock];
    float* pfReal_BlockFFTCurrent = new float[BlockLength + 1];
    float* pfImage_BlockFFTCurrent = new float[BlockLength + 1];
    CFft* pforFFT = 0;
    CFft::complex_t* pfComplexNum = 0;
    float* pfReal_FFT = new float[BlockLength + 1];
    float* pfImage_FFT = new float[BlockLength + 1];
    float* pf_IFFT = new float[2 * BlockLength]{ 0 };
    int IdxBlock_Write = 0;
    float** ppfReal_IRFreq = new float* [NumofBlock];
    float** ppfImage_IRFreq = new float* [NumofBlock];
    for (int i = 0; i < NumofBlock; i++)
    {
        ppfReal_IRFreq[i] = new float[BlockLength + 1]{ 0 };
        ppfImage_IRFreq[i] = new float[BlockLength + 1]{ 0 };
        ppfOutputBlockBuffer[i] = new float[BlockLength] {0};
        for (int j = 0; j < BlockLength; j++)
        {
            if (i * BlockLength + j < impulseResponse.size())
                pf_IFFT[j] = impulseResponse[i * BlockLength + j];
            else
                pf_IFFT[j] = 0;
        }

        for (int j = BlockLength; j < 2 * BlockLength; j++)
            pf_IFFT[j] = 0;

        pforFFT->doFft(pfComplexNum, pf_IFFT);
        pforFFT->splitRealImag(ppfReal_IRFreq[i], ppfImage_IRFreq[i], pfComplexNum);
    }

    for (int i = 0; i < length; i++)
    {
        pfInputBlockBuffer[IdxWrite + BlockLength] = input[i];

        output[i] = ppfOutputBlockBuffer[BlockIdxRead][IdxWrite];

        IdxWrite++;

        if (IdxWrite == BlockLength)
        {
            IdxWrite = 0;
            for (int j = 0; j < BlockLength; j++)
            {
                ppfOutputBlockBuffer[BlockIdxRead][j] = 0;
            }
            pforFFT->doFft(pfComplexNum, pfInputBlockBuffer);
            pforFFT->splitRealImag(pfReal_BlockFFTCurrent, pfImage_BlockFFTCurrent, pfComplexNum);
            for (int j = 0; j < NumofBlock; j++)
            {
                for (int i = 0; i <= BlockLength; i++)
                {
                    pfReal_FFT[i] = (pfReal_BlockFFTCurrent[i] * ppfReal_IRFreq[j][i] - pfImage_BlockFFTCurrent[i] * ppfImage_IRFreq[j][i]) * 2 * BlockLength;
                    pfImage_FFT[i] = (pfReal_BlockFFTCurrent[i] * ppfImage_IRFreq[j][i] + pfImage_BlockFFTCurrent[i] * ppfReal_IRFreq[j][i]) * 2 * BlockLength;
                }
                pforFFT->mergeRealImag(pfComplexNum, pfReal_FFT, pfImage_FFT);
                pforFFT->doInvFft(pf_IFFT, pfComplexNum);
                const int l_iWriteBlockNum = (IdxBlock_Write + j) % NumofBlock;
                for (int k = 0; k < BlockLength; k++)
                {
                    ppfOutputBlockBuffer[l_iWriteBlockNum][k] += pf_IFFT[k + BlockLength];
                }
            }
            for (int j = 0; j < BlockLength; j++)
            {
                pfInputBlockBuffer[j] = pfInputBlockBuffer[j + BlockLength];
            }

            BlockIdxRead = IdxBlock_Write;
            IdxBlock_Write = (IdxBlock_Write + 1) % NumofBlock;
        }
    }

    for (int i = 0; i < NumofBlock; i++)
    {
        delete[] ppfOutputBlockBuffer[i];
        delete[] ppfReal_IRFreq[i];
        delete[] ppfImage_IRFreq[i];
    }
    delete[] ppfReal_IRFreq;
    delete[] ppfImage_IRFreq;
    delete[] pfInputBlockBuffer;
    delete[] ppfOutputBlockBuffer;

    ppfReal_IRFreq = 0;
    ppfImage_IRFreq = 0;
    pfInputBlockBuffer = 0;
    ppfOutputBlockBuffer = 0;

    delete[] pf_IFFT;
    delete[] pfReal_FFT;
    delete[] pfImage_FFT;
    delete[] pfReal_BlockFFTCurrent;
    delete[] pfImage_BlockFFTCurrent;
    pf_IFFT = 0;
    pfReal_FFT = 0;
    pfImage_FFT = 0;


    delete[] pfComplexNum;
    pfComplexNum = 0;

    CFft::destroyInstance(pforFFT);
    pforFFT = 0;


}



Error_t CFastConv::flushBuffer(float* pfOutputBuffer) {
    // NOTE: The tail is always the length of the impulse response minus one,
    // even if the user has provided fewer than `impulseResponse.size()` input samples.
    // TODO: Maybe just implement this in terms of `process`?
    for (int i = 0; i < impulseResponse.size() - 1; i++) {
        history->putPostInc(0);
        float acc = 0;
        for (int j = 0; j < impulseResponse.size(); j++) {
            acc += impulseResponse[j] * history->get(-j);
        }
        pfOutputBuffer[i] = acc;
        history->getPostInc();
    }
    return Error_t::kNoError;
}
