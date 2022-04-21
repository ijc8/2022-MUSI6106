#if !defined(__FastConv_HEADER_INCLUDED__)
#define __FastConv_HEADER_INCLUDED__

#pragma once

#include <memory>
#include <vector>

#include "RingBuffer.h"
#include "ErrorDef.h"
#include "Fft.h"

// Abstract base class. Allows polymorphism in CFastConv class,
// which wraps both convolution methods (time-domain & frequency-domain).
class Convolution {
public:
    virtual ~Convolution() {};
    virtual void process(float *output, const float *input, int length) = 0;
    virtual int getTailLength() const = 0;
};

class TimeConvolution: public Convolution {
public:
    TimeConvolution(const float *impulseResponse, int length);
    void process(float *output, const float *input, int length) override;
    int getTailLength() const override;
private:
    std::vector<float> impulseResponse;
    CRingBuffer<float> history;
};

class FreqConvolution: public Convolution {
public:
    FreqConvolution(const float *impulseResponse, int length, int blockLength);
    ~FreqConvolution();
    void process(float *output, const float *input, int length) override;
    int getTailLength() const override;
private:
    int indexInBlock = 0;
    int inputBlockIndex = 0;
    CFft *fft = nullptr;
    // NOTE: Construction order matters;
    // vectors depend on these getting initialized first in the initialization list.
    int blockLength, tailLength, numBlocks;
    // `inputBlocks` contains the spectra of the last few blocks, stored contiguously.
    // It also contains the time-domain samples of the next incoming block.
    std::vector<float> inputBlocks, outputBlock, saved, impulseResponseBlocks;
};

/*! \brief interface for fast convolution
*/
class CFastConv {
public:
    enum ConvCompMode_t {
        kTimeDomain,
        kFreqDomain,

        kNumConvCompModes
    };

    CFastConv();
    virtual ~CFastConv();

    /*! initializes the class with the impulse response and the block length
    \param pfImpulseResponse impulse response samples (mono only)
    \param iLengthOfIr length of impulse response
    \param iBlockLength processing block size
    \return Error_t
    */
    Error_t init(float* pfImpulseResponse, int iLengthOfIr, int iBlockLength = 8192, ConvCompMode_t eCompMode = kFreqDomain);

    /*! resets all internal class members
    \return Error_t
    */
    Error_t reset();

    /*! computes the output with reverb
    \param pfOutputBuffer (mono)
    \param pfInputBuffer (mono)
    \param iLengthOfBuffers can be anything from 1 sample to 10000000 samples
    \return Error_t
    */
    Error_t process(float* pfOutputBuffer, const float* pfInputBuffer, int iLengthOfBuffers);

    /*! return the length of the tail; this is the minimum size of the buffer passed to `flushBuffer`. */
    int getTailLength() const;

    /*! return the 'tail' after processing has finished (identical to feeding in zeros)
    \param pfOutputBuffer (mono)
    \return Error_t
    */
    Error_t flushBuffer(float* pfOutputBuffer);

private:
    std::unique_ptr<Convolution> conv;
};


#endif
