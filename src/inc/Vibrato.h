#if !defined(__Vibrato_hdr__)
#define __Vibrato_hdr__

#include <memory>
#include <vector>

#include "LFO.h"
#include "RingBuffer.h"
#include "ErrorDef.h"

// TODO: Explain design here.

class Vibrato {
public:
    Vibrato(float sampleRate, float maxDepth, int numChannels);

    void setFrequency(float frequency);
    Error_t setDepth(float depth);

    float getFrequency() const;
    float getDepth() const;

    void process(float **input, float **output, int numFrames);

private:
    float sampleRate;
    LFO lfo;
    std::vector<std::unique_ptr<RingBuffer<float> > > delayLines;
};

#endif // __Vibrato_hdr__
