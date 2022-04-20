
#if !defined(__FastConv_HEADER_INCLUDED__)
#define __FastConv_HEADER_INCLUDED__

#pragma once

#include <memory>
#include <vector>

#include "RingBuffer.h"
#include "ErrorDef.h"

/*! \brief interface for fast convolution
*/
class CFastConv
{
public:
    enum ConvCompMode_t
    {
        kTimeDomain,
        kFreqDomain,

        kNumConvCompModes
    };

    CFastConv(void);
    virtual ~CFastConv(void);

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
    Error_t reset ();

    /*! computes the output with reverb
    \param pfOutputBuffer (mono)
    \param pfInputBuffer (mono)
    \param iLengthOfBuffers can be anything from 1 sample to 10000000 samples
    \return Error_t
    */
    Error_t process(float* pfOutputBuffer, const float* pfInputBuffer, int iLengthOfBuffers);

    int getTailLength();

    /*! return the 'tail' after processing has finished (identical to feeding in zeros)
    \param pfOutputBuffer (mono)
    \return Error_t
    */
    Error_t flushBuffer(float* pfOutputBuffer);

private:
    ConvCompMode_t mode;
    // Question 1: Can we assume the pfImpulseResponse pointer the user gives us remains valid?
    // Or should we copy their provided data into our own buffer as a precaution?
    // Question 2: Am I correct in thinking iBlockLength is unused in time-domain mode?
    std::vector<float> impulseResponse;
    std::unique_ptr<CRingBuffer<float>> history;

    void processTimeDomain(float *output, const float *input, int length);
    void processFreqDomain(float *output, const float *input, int length);

    // Only used for freq domain:
    int blockLength;
    std::unique_ptr<CRingBuffer<float>> inputBuffer, outputBuffer;
    std::unique_ptr<CRingBuffer<std::vector<float>>> inputBlockHistory;
    std::vector<std::vector<float>> impulseResponseBlocks;
    std::vector<float> saved;
};


#endif
