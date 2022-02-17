#if !defined(__CombFilter_hdr__)
#define __CombFilter_hdr__

#include "ErrorDef.h"
#include "RingBuffer.h"

class CCombFilterBase {
public:
    CCombFilterBase(float maxDelay, float sampleRate);
    Error_t setGain(float gain);
    virtual Error_t setDelay(float delay);
    float getGain();
    float getDelay();
    virtual Error_t process(float *input, float *output, int numFrames) = 0;
protected:
    float sampleRate;
    RingBuffer<float> delayline;
    float gain = 0.5;
};

class FIRFilter : public CCombFilterBase {
public:
    using CCombFilterBase::CCombFilterBase;
    Error_t process(float *input, float *output, int numFrames) override;
};

class IIRFilter : public CCombFilterBase {
public:
    using CCombFilterBase::CCombFilterBase;
    Error_t setDelay(float delay) override;
    Error_t process(float *input, float *output, int numFrames) override;
};

#endif // #if !defined(__CombFilter_hdr__)
