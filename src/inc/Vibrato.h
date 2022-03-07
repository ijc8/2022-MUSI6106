#if !defined(__Vibrato_hdr__)
#define __Vibrato_hdr__

#include <memory>
#include <vector>

#include "LFO.h"
#include "RingBuffer.h"
#include "ErrorDef.h"

// Our design consists of a Vibrato class that supports multichannel,
// with as many delay lines as channels, controlled by a single LFO.
// We provide methods to get and set parameters directly,
// which is more direct and avoids a potential source of error (invalid parameter type).
// All parameter values are expressed in Hz or seconds, as appropriate.

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
    float maxDepth;
    LFO lfo;
    std::vector<std::unique_ptr<RingBuffer<float>>> delayLines;
};

#endif // __Vibrato_hdr__
