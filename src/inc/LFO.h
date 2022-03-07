#if !defined(__LFO_hdr__)
#define __LFO_hdr__

#include "RingBuffer.h"

class LFO {
public:
    LFO(float sampleRate, int resolution = 4096);
    float getAmplitude() const;
    float getFrequency() const;
    void setAmplitude(float amplitude);
    void setFrequency(float frequency);
    void reset();
    float process();
private:
    float sampleRate;
    float amplitude = 0;
    float frequency = 0;
    float index = 0;
    RingBuffer<float> wavetable;
};

#endif // __LFO_hdr__
