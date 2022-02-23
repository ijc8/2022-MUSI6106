#if !defined(__LFO_hdr__)
#define __LFO_hdr__

#include "RingBuffer.h"

class LFO {
public:
    LFO(float sampleRate, int resolution = 4096);
    float getFrequency() const;
    void setFrequency(float frequency);
    float process();
private:
    float sampleRate;
    float frequency = 0;
    float index = 0;
    RingBuffer<float> wavetable;
};

#endif // __LFO_hdr__
