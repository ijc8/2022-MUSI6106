
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "FastConv.h"

using std::cout;
using std::endl;

void showClInfo() {
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout << endl;
    return;
}

int main(int argc, char* argv[]) {
    std::string sInputFilePath, sOutputFilePath;
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

    showClInfo();

    // command line args
    if (argc < 4) {
        cout << "Usage: " << argv[0] << " <input file> <output file> <delay in seconds>" << endl;
        return -1;
    }
    sInputFilePath = argv[1];
    sOutputFilePath = argv[2];
    delayInSec = atof(argv[3]);

    // open audio files
    CAudioFileIf::create(phAudioFile);
    CAudioFileIf::create(phAudioOutputFile);

    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    phAudioFile->getFileSpec(stFileSpec);
    phAudioOutputFile->openFile(sOutputFilePath, CAudioFileIf::kFileWrite, &stFileSpec);
    iNumChannels = stFileSpec.iNumChannels;
    if (!phAudioFile->isOpen()) {
        cout << "Input file open error!";
        CAudioFileIf::destroy(phAudioFile);
        CAudioFileIf::destroy(phAudioOutputFile);
        return -1;
    } else if (!phAudioOutputFile->isOpen()) {
        cout << "Output file cannot be initialized!" << endl;
        CAudioFileIf::destroy(phAudioFile);
        CAudioFileIf::destroy(phAudioOutputFile);
        return -1;
    } else if (iNumChannels != 1) {
        cout << "Only mono audio is supported." << endl;
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
    fastConv.init(impulseResponse, impulseResponseLength, 0, CFastConv::kTimeDomain);

    // processing
    clock_t processingTime = 0;
    while (!phAudioFile->isEof()) {
        phAudioFile->readData(ppfInputAudio, iNumFrames);
        clock_t start = clock();
        fastConv.process(ppfOutputAudio[0], ppfInputAudio[0], iNumFrames);
        processingTime += clock() - start;
        phAudioOutputFile->writeData(ppfOutputAudio, iNumFrames);
    }
    cout << "reading/writing done in: \t" << (float)(clock() - time) / CLOCKS_PER_SEC << " seconds." << endl;
    cout << "time spent in process(): \t" << ((float)processingTime / CLOCKS_PER_SEC) << " seconds." << endl;

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

    // all done
    return 0;
}
