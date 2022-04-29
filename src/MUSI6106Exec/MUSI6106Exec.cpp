
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "FastConv.h"

int main(int argc, char* argv[]) {
    std::string sInputFilePath, sOutputFilePath, mode;
    float delayInSec;
    int delayInSamples;

    static const int kBlockSize = 1024;
    long long iNumFrames = kBlockSize;
    int iNumChannels;

    clock_t time = 0;

    float **ppfInputAudio = 0;
    float **ppfOutputAudio = 0;

    CAudioFileIf *phAudioFile = 0;
    CAudioFileIf *phAudioOutputFile = 0;

    CAudioFileIf::FileSpec_t stFileSpec;

    // command line args
    if (argc < 5) {
        std::cout << "Usage: " << argv[0] << " <input file> <output file> <delay in seconds> <time|freq>" << std::endl;
        return -1;
    }
    sInputFilePath = argv[1];
    sOutputFilePath = argv[2];
    delayInSec = atof(argv[3]);
    mode = argv[4];

    // open audio files
    CAudioFileIf::create(phAudioFile);
    CAudioFileIf::create(phAudioOutputFile);

    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    phAudioFile->getFileSpec(stFileSpec);
    phAudioOutputFile->openFile(sOutputFilePath, CAudioFileIf::kFileWrite, &stFileSpec);
    iNumChannels = stFileSpec.iNumChannels;
    if (!phAudioFile->isOpen()) {
        std::cout << "Input file open error!";
        CAudioFileIf::destroy(phAudioFile);
        CAudioFileIf::destroy(phAudioOutputFile);
        return -1;
    } else if (!phAudioOutputFile->isOpen()) {
        std::cout << "Output file cannot be initialized!" << std::endl;
        CAudioFileIf::destroy(phAudioFile);
        CAudioFileIf::destroy(phAudioOutputFile);
        return -1;
    } else if (iNumChannels != 1) {
        std::cout << "Only mono audio is supported." << std::endl;
        CAudioFileIf::destroy(phAudioFile);
        CAudioFileIf::destroy(phAudioOutputFile);
        return -1;
    }
    delayInSamples = delayInSec * stFileSpec.fSampleRateInHz;

    // allocate memory
    ppfInputAudio = new float* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfInputAudio[i] = new float[kBlockSize];

    ppfOutputAudio = new float* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfOutputAudio[i] = new float[kBlockSize];

    // Setup reverb
    // With this IR, this is essentially a very expensive comb filter...
    int impulseResponseLength = delayInSamples + 1;
    float impulseResponse[impulseResponseLength] = {0};
    impulseResponse[0] = 1;
    impulseResponse[impulseResponseLength - 1] = 1;
    CFastConv fastConv;
    CFastConv::ConvCompMode_t convMode = mode == "time" ? CFastConv::kTimeDomain : CFastConv::kFreqDomain;
    fastConv.init(impulseResponse, impulseResponseLength, kBlockSize, convMode);

    // processing
    clock_t processingTime = 0;
    if (convMode == CFastConv::kFreqDomain) {
        // Dump first block of samples, which are silent due to latency.
        phAudioFile->readData(ppfInputAudio, iNumFrames);
        clock_t start = clock();
        fastConv.process(ppfOutputAudio[0], ppfInputAudio[0], iNumFrames);
        processingTime += clock() - start;
    }
    while (!phAudioFile->isEof()) {
        phAudioFile->readData(ppfInputAudio, iNumFrames);
        clock_t start = clock();
        fastConv.process(ppfOutputAudio[0], ppfInputAudio[0], iNumFrames);
        processingTime += clock() - start;
        phAudioOutputFile->writeData(ppfOutputAudio, iNumFrames);
    }
    // Flush reverb tail.
    int tailLength = fastConv.getTailLength();
    float tail[tailLength];
    clock_t start = clock();
    fastConv.flushBuffer(tail);
    processingTime += clock() - start;
    float *wrapper[] = { tail };
    phAudioOutputFile->writeData(wrapper, tailLength);

    std::cout << "reading/writing done in: \t" << (float)(clock() - time) / CLOCKS_PER_SEC << " seconds." << std::endl;
    std::cout << "time in process/flushBuffer: \t" << ((float)processingTime / CLOCKS_PER_SEC) << " seconds." << std::endl;

    // clean-up
    CAudioFileIf::destroy(phAudioFile);
    CAudioFileIf::destroy(phAudioOutputFile);

    for (int i = 0; i < stFileSpec.iNumChannels; i++) {
        delete[] ppfInputAudio[i];
        delete[] ppfOutputAudio[i];
    }
    delete[] ppfInputAudio;
    delete[] ppfOutputAudio;
    ppfInputAudio = 0;
    ppfOutputAudio = 0;

    return 0;
}
