#if !defined(__Vibrato_hdr__)
#define __Vibrato_hdr__

#include "LFO.h"
#include "RingBuffer.h"
#include "ErrorDef.h"

class Vibrato {
public:
    Vibrato(float sampleRate, float maxDepth, int iNumChannels);

    void setFrequency(float frequency);
    Error_t setDepth(float depth);

    float getFrequency() const;
    float getDepth() const;

    void process(float **input, float **output, int numFrames);

private:
    float m_fsampleRate;
    float m_fdepth;
    int m_iNumChannels;
    LFO lfo;
    RingBuffer<float> delayLine;
//    RingBuffer<float> **mpp_delayLine;
};

#endif // __Vibrato_hdr__
