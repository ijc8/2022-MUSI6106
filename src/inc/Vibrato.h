#if !defined(__Vibrato_hdr__)
#define __Vibrato_hdr__

#include "LFO.h"
#include "RingBuffer.h"
#include "ErrorDef.h"

class Vibrato {
public:
    Vibrato(float sampleRate, float maxDepth);

    void setFrequency(float frequency);
    void setDepth(float depth);

    float getFrequency() const;
    float getDepth() const;

    void process(float **input, float **output, int numFrames);

private:
    float m_fsampleRate;
    float m_fdepth;
    LFO lfo;
    RingBuffer<float> delayLine;
//    RingBuffer<float> **mpp_delayLine;
};

#endif // __Vibrato_hdr__
