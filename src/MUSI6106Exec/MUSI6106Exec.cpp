
#include <ctime>
#include <iostream>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "Vibrato.h"
#include "portaudio.h"

using std::cout;
using std::endl;

// local function declarations
void showClInfo();

typedef struct PortAudioUserData {
    CAudioFileIf *phAudioFile = 0;
    CVibrato *pCVibrato = 0;
    float **ppfInputAudio = 0;
    float **ppfOutputAudio = 0;
    long long iNumFrames;
    int iNumChannels;
} PortAudioUserData;

// This routine will be called by the PortAudio engine when audio is needed.
static int patestCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags, void *userData) {
    PortAudioUserData *data = (PortAudioUserData *)userData;
    float *out = (float *)outputBuffer;

    (void)inputBuffer;
    (void)statusFlags;
    (void)timeInfo;

    // processing
    if (!data->phAudioFile->isEof()) {
        data->phAudioFile->readData(data->ppfInputAudio, data->iNumFrames);

        data->pCVibrato->process(data->ppfInputAudio, data->ppfOutputAudio,
                                 data->iNumFrames);

        // fill in PortAudio output with Vibrato output.
        for (int i = 0; i < data->iNumFrames; i++) {
            for (int c = 0; c < data->iNumChannels; c++) {
                *out++ = data->ppfOutputAudio[c][i];
            }
        }

        // fill in remainder of frames for empty audio output.
        for (int i = data->iNumFrames; i < framesPerBuffer; i++) {
            for (int c = 0; c < data->iNumChannels; c++) {
                *out++ = 0.f;
            }
        }
    } else {
        // fill PortAudio output with empty audio output (end of file).
        for (int i = 0; i < framesPerBuffer; i++) {
            for (int c = 0; c < data->iNumChannels; c++) {
                *out++ = 0.f;
            }
        }
        return paComplete;
    }

    return paContinue;
}

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char *argv[]) {

    std::string sInputFilePath; //!< input file

    static const int kBlockSize = 1024;

    float fModFrequencyInHz;
    float fModWidthInSec;

    CAudioFileIf::FileSpec_t stFileSpec;

    PaStream *pPaStream;
    PaStreamParameters stStreamParameters;
    PaError err;
    PortAudioUserData stUserData;

    showClInfo();

    // command line args
    if (argc < 4) {
        fModWidthInSec = 0.f;
        if (argc < 3) {
            fModFrequencyInHz = 0.f;
        }
        if (argc < 2) {
            sInputFilePath = "input.wav";
        }
    } else {
        sInputFilePath = argv[1];
        fModFrequencyInHz = atof(argv[2]);
        fModWidthInSec = atof(argv[3]);
    }

    ///////////////////////////////////////////////////////////////////////////
    // audio file
    CAudioFileIf::create(stUserData.phAudioFile);

    stUserData.phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    stUserData.phAudioFile->getFileSpec(stFileSpec);
    stUserData.iNumChannels = stFileSpec.iNumChannels;

    if (!stUserData.phAudioFile->isOpen()) {
        cout << "Input file open error!";

        CAudioFileIf::destroy(stUserData.phAudioFile);
        return -1;
    }

    stUserData.iNumFrames = kBlockSize;

    ////////////////////////////////////////////////////////////////////////////
    // vibrato instance
    CVibrato::create(stUserData.pCVibrato);
    stUserData.pCVibrato->init(fModWidthInSec, stFileSpec.fSampleRateInHz,
                               stUserData.iNumChannels);

    // allocate memory
    stUserData.ppfInputAudio = new float *[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        stUserData.ppfInputAudio[i] = new float[kBlockSize];

    stUserData.ppfOutputAudio = new float *[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        stUserData.ppfOutputAudio[i] = new float[kBlockSize];

    // Set parameters of vibrato
    stUserData.pCVibrato->setParam(CVibrato::kParamModFreqInHz,
                                   fModFrequencyInHz);
    stUserData.pCVibrato->setParam(CVibrato::kParamModWidthInS, fModWidthInSec);

    ////////////////////////////////////////////////////////////////////////////
    // initialize PortAudio
    err = Pa_Initialize();
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    }

    // setup audio output
    stStreamParameters.device =
        Pa_GetDefaultOutputDevice(); /* default output device */
    stStreamParameters.channelCount = stFileSpec.iNumChannels;
    stStreamParameters.sampleFormat = paFloat32;
    stStreamParameters.suggestedLatency =
        Pa_GetDeviceInfo(stStreamParameters.device)->defaultLowOutputLatency;
    stStreamParameters.hostApiSpecificStreamInfo = NULL;

    // setup stream
    err = Pa_OpenStream(&pPaStream, NULL, &stStreamParameters,
                        stFileSpec.fSampleRateInHz, kBlockSize, paClipOff,
                        patestCallback, &stUserData);

    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    }

    err = Pa_StartStream(pPaStream);
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    }

    // Sleep until stream is finished.
    while (Pa_IsStreamActive(pPaStream)) {
        // Pa_Sleep(100);
        float width;
        std::cout << "> ";
        std::cin >> width;
        stUserData.pCVibrato->setParam(CVibrato::kParamModWidthInS, width);
    }

    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    err = Pa_StopStream(pPaStream);
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    }

    err = Pa_CloseStream(pPaStream);
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    }

    err = Pa_Terminate();
    if (err != paNoError) {
        printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    }

    CAudioFileIf::destroy(stUserData.phAudioFile);
    CVibrato::destroy(stUserData.pCVibrato);

    for (int i = 0; i < stFileSpec.iNumChannels; i++) {
        delete[] stUserData.ppfInputAudio[i];
        delete[] stUserData.ppfOutputAudio[i];
    }
    delete[] stUserData.ppfInputAudio;
    delete[] stUserData.ppfOutputAudio;
    stUserData.ppfInputAudio = 0;
    stUserData.ppfOutputAudio = 0;

    // all done
    return 0;
}

void showClInfo() {
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout << endl;

    return;
}
