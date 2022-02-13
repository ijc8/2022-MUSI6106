
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"

using std::cout;
using std::endl;

void showClInfo() {
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout << endl;
}

void showUsage(char *progName) {
    cout << "Usage: " << progName << " <input wav> <output wav> <delay in seconds> <feedforward/back gain>" << endl;
}

// main function
int main(int argc, char *argv[]) {
    std::string inputPath, outputPath;  //!< file paths
    float delay, gain;
    static const int blockSize = 1024;

    clock_t time = 0;

    float **audioData = 0;

    CAudioFileIf *inputAudioFile, *outputAudioFile;
    CAudioFileIf::FileSpec_t fileSpec;

    showClInfo();

    // parse command line arguments
    if (argc == 1) {
        cout << "Testing mode (TODO!)" << endl;
        return -1;
    } else if (argc < 5) {
        showUsage(argv[0]);
        return -1;
    } else {
        inputPath = argv[1];
        outputPath = argv[2];
        try {
            delay = std::stof(argv[3]);
            gain = std::stof(argv[4]);
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
    filter->init(CCombFilterIf::kCombFIR, delay, fileSpec.fSampleRateInHz, fileSpec.iNumChannels);
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

        cout << "\r" << "reading and writing";

        outputAudioFile->writeData(audioData, iNumFrames);
    }

    cout << "\nreading/writing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

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
