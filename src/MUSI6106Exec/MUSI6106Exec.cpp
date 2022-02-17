
#include <iostream>
#include <cmath>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"

using std::cout;
using std::endl;

// Helper for testing.
int expectIdx;

void startTest() {
    expectIdx = 0;
}

void expect(bool b) {
    if (!b) throw expectIdx;
    expectIdx++;
}

// "Close enough" value for floating point expectations.
const float epsilon = 1e-5;

inline float randf() {
    return (float)rand() / RAND_MAX;
}

void testFIRInterference() {
    // Feed in sine wave with period = delay * 2; should cancel with itself.
    const float sampleRate = 44100;
    const float delay = 0.1;
    const float freq = 1 / (delay * 2);

    CCombFilterIf *filter;
    CCombFilterIf::create(filter);
    filter->init(CCombFilterIf::kCombFIR, delay, sampleRate, 1);
    filter->setParam(CCombFilterIf::kParamDelay, delay);
    filter->setParam(CCombFilterIf::kParamGain, 1.0);

    for (int i = 0; i < delay * sampleRate * 10; i++) {
        float in = sinf(2*M_PI*freq*i/sampleRate);
        float *ins = &in;
        float out;
        float *outs = &out;
        filter->process(&ins, &outs, 1);
        if (i >= delay * sampleRate) {
            expect(fabsf(out) < epsilon);
        }
    }

    CCombFilterIf::destroy(filter);
}

void testIIRInterference() {
    // Feed in sine wave with period = delay; magnitude should increase with every cycle.
    const float sampleRate = 44100;
    const float delay = 0.1;
    const float freq = 1 / delay;

    CCombFilterIf *filter;
    CCombFilterIf::create(filter);
    filter->init(CCombFilterIf::kCombIIR, delay, sampleRate, 1);
    filter->setParam(CCombFilterIf::kParamDelay, delay);
    filter->setParam(CCombFilterIf::kParamGain, 1.0);

    for (int i = 0; i < delay * sampleRate * 10; i++) {
        float in = sinf(2*M_PI*freq*i/sampleRate);
        float *ins = &in;
        float out;
        float *outs = &out;
        filter->process(&ins, &outs, 1);
        if (i >= delay * sampleRate) {
            // Due to feedback loop, signal should keep constructively interfering with delay, producing larger and larger values.
            // Output should be delayCycle * input, where delayCycle is how many times we've gone around delay (starting from 1).
            int delayCycle = i / (delay * sampleRate) + 1;
            expect(fabsf(out / delayCycle - in) < epsilon);
        }
    }

    CCombFilterIf::destroy(filter);
}

void testBlockSize() {
    // For simplicity, the other tests process one sample at a time.
    // In contrast, this test specifically checks for handling of variable block sizes.
    const float sampleRate = 12345;
    const float delay = 0.1;

    for (auto type : {CCombFilterIf::kCombFIR, CCombFilterIf::kCombIIR}) {
        CCombFilterIf *filter;
        CCombFilterIf::create(filter);

        const int length = 8192;
        float input[length];
        float outputA[length] = {0};
        float outputB[length] = {0};

        // Generate random input signal.
        srand(1234);
        for (int i = 0; i < length; i++) {
            input[i] = randf();
        }

        // First, compute the output in one go.
        float *ins = input;
        float *outs = outputA;
        filter->init(type, delay, sampleRate, 1);
        filter->process(&ins, &outs, length);

        // Then, compute the output in many smaller blocks with variable size (from 0-1024).
        filter->reset();
        filter->init(type, delay, sampleRate, 1);
        for (int i = 0; i < length;) {
            int blockSize = std::min((int)(randf() * 1024), length - i);
            float *ins = &input[i];
            float *outs = &outputB[i];
            filter->process(&ins, &outs, blockSize);
            i += blockSize;
        }

        // Ensure outputs are identical.
        for (int i = 0; i < length; i++) {
            expect(outputA[i] == outputB[i]);
        }

        CCombFilterIf::destroy(filter);
    }
}

void testSilence() {
    // Test that output is silent for silent input (signal of zeros).
    const float sampleRate = 12345;
    const float delay = 0.1;

    for (auto type : {CCombFilterIf::kCombFIR, CCombFilterIf::kCombIIR}) {
        CCombFilterIf *filter;
        CCombFilterIf::create(filter);
        // Choice of delay, gain, sample rate should be irrelevant for this test.
        filter->init(type, delay, sampleRate, 1);

        for (int i = 0; i < delay * sampleRate * 10; i++) {
            float in = 0;
            float *ins = &in;
            float out;
            float *outs = &out;
            filter->process(&ins, &outs, 1);
            expect(out == 0);
        }

        CCombFilterIf::destroy(filter);
    }
}

void testNoDelay() {
    // Test behavior when delay is set to zero.
    const float sampleRate = 12345;
    const float maxDelay = 0.1;

    // For FIR filter, output should be merely scaled when delay is zero.
    CCombFilterIf *filter;
    CCombFilterIf::create(filter);
    filter->init(CCombFilterIf::kCombFIR, maxDelay, sampleRate, 1);
    // Set delay to 0.
    filter->setParam(CCombFilterIf::kParamDelay, 0);
    float expectedGain = 1 + filter->getParam(CCombFilterIf::kParamGain);

    srand(1234);
    for (int i = 0; i < maxDelay * sampleRate * 10; i++) {
        float in = randf();
        float *ins = &in;
        float out;
        float *outs = &out;
        filter->process(&ins, &outs, 1);
        // Since delay is 0, we expect `out = in + gain * in`.
        expect(fabsf(out / expectedGain - in) < epsilon);
    }

    // For IIR filter, a delay of zero is invalid, as it would create a zero-delay feedback loop.
    // Check for the expected error.
    filter->reset();
    filter->init(CCombFilterIf::kCombIIR, maxDelay, sampleRate, 1);
    expect(filter->setParam(CCombFilterIf::kParamDelay, 0) == Error_t::kFunctionInvalidArgsError);

    CCombFilterIf::destroy(filter);
}

void testMultichannel() {
    // Test multichannel support. Ensure that filter output is correct for each channel.
    float input[2][10] = {
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
        {1, 0, 1, 0, 1, 0, 1, 0, 1, 0},
    };
    float output[2][10];
    float *ins[2] = {input[0], input[1]};
    float *outs[2] = {output[0], output[1]};

    const float sampleRate = 10;
    const float gain = 0.1;
    // Delay of one sample.
    const float delay = 1 / sampleRate;

    CCombFilterIf *filter;
    CCombFilterIf::create(filter);
    filter->init(CCombFilterIf::kCombFIR, delay, 10, 2);
    filter->setParam(CCombFilterIf::kParamDelay, delay);
    filter->setParam(CCombFilterIf::kParamGain, gain);

    filter->process(ins, outs, 10);
    for (int i = 0; i < 2; i++) {
        for (int j = 1; j < 10; j++) {
            float expected = input[i][j] + gain * input[i][j-1];
            expect(fabsf(output[i][j] - expected) < epsilon);
        }
    }

    CCombFilterIf::destroy(filter);
}

int runTests() {
    cout << "Running tests." << endl;
    auto tests = {testFIRInterference, testIIRInterference, testBlockSize, testSilence, testNoDelay, testMultichannel};
    int passed = 0;
    for (int i = 0; i < tests.size(); i++) {
        try {
            startTest();
            tests.begin()[i]();
            cout << "Test " << i << " passed ✓" << endl;
            passed++;
        } catch (int &expectIdx) {
            cout << "Test " << i << " failed ✗ (at expect " << expectIdx << ")" << endl;
        }
    }
    cout << "Tests passed: " << passed << "/" << tests.size() << endl;
    return passed != tests.size();
}


void showClInfo() {
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout << endl;
}

void showUsage(const char *progName) {
    cout << "Usage: " << progName << " <input wav> <output wav> [fir|iir] <delay in seconds> <feedforward/back gain>" << endl;
}

// main function
int main(int argc, char *argv[]) {
    std::string inputPath, outputPath;  //!< file paths
    float delay, gain;
    std::string fir = "fir", iir = "iir";
    CCombFilterIf::CombFilterType_t type;
    static const int blockSize = 1024;

    clock_t time = 0;

    float **audioData = 0;

    CAudioFileIf *inputAudioFile, *outputAudioFile;
    CAudioFileIf::FileSpec_t fileSpec;

    showClInfo();

    // parse command line arguments
    if (argc == 1) {
        return runTests();
    } else if (argc < 5 || !(argv[3] == fir || argv[3] == iir)) {
        showUsage(argv[0]);
        return -1;
    } else {
        inputPath = argv[1];
        outputPath = argv[2];
        type = argv[3] == fir ? CCombFilterIf::kCombFIR : CCombFilterIf::kCombIIR;
        try {
            delay = std::stof(argv[4]);
            gain = std::stof(argv[5]);
        } catch (const std::exception &exc) {
            cout << "error: " << exc.what() << endl;
            showUsage(argv[0]);
            return -1;
        }
    }

    // open the input wave file
    CAudioFileIf::create(inputAudioFile);
    inputAudioFile->openFile(inputPath, CAudioFileIf::kFileRead);
    if (!inputAudioFile->isOpen()) {
        cout << "Wave file open error!";
        CAudioFileIf::destroy(inputAudioFile);
        return -1;
    }
    inputAudioFile->getFileSpec(fileSpec);

    // open the output wave file
    CAudioFileIf::create(outputAudioFile);
    outputAudioFile->openFile(outputPath, CAudioFileIf::kFileWrite, &fileSpec);
    if (!outputAudioFile->isOpen()) {
        cout << "Wave file open error!";
        CAudioFileIf::destroy(inputAudioFile);
        CAudioFileIf::destroy(outputAudioFile);
        return -1;
    }

    // allocate memory
    audioData = new float*[fileSpec.iNumChannels];
    for (int i = 0; i < fileSpec.iNumChannels; i++)
        audioData[i] = new float[blockSize];

    if (audioData == 0) {
        CAudioFileIf::destroy(inputAudioFile);
        CAudioFileIf::destroy(outputAudioFile);
        return -1;
    } else if (audioData[0] == 0) {
        CAudioFileIf::destroy(inputAudioFile);
        CAudioFileIf::destroy(outputAudioFile);
        return -1;
    }

    CCombFilterIf *filter;
    CCombFilterIf::create(filter);
    filter->init(type, delay, fileSpec.fSampleRateInHz, fileSpec.iNumChannels);
    filter->setParam(CCombFilterIf::kParamDelay, delay);
    filter->setParam(CCombFilterIf::kParamGain, gain);

    time = clock();

    // get audio data and write it to the output text file (one column per channel)
    while (!inputAudioFile->isEof()) {
        // set block length variable
        long long iNumFrames = blockSize;

        // read data (iNumOfFrames might be updated!)
        inputAudioFile->readData(audioData, iNumFrames);

        // apply filter to audio
        filter->process(audioData, audioData, iNumFrames);

        cout << "\r" << "processing...";
        outputAudioFile->writeData(audioData, iNumFrames);
    }

    cout << "\ndone in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

    // clean-up (close files and free memory)
    CCombFilterIf::destroy(filter);
    CAudioFileIf::destroy(inputAudioFile);
    CAudioFileIf::destroy(outputAudioFile);

    for (int i = 0; i < fileSpec.iNumChannels; i++)
        delete[] audioData[i];
    delete[] audioData;
    audioData = 0;

    // all done
    return 0;
}
