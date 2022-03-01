
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "RingBuffer.h"
#include "Synthesis.h"
#include "LFO.h"
#include "Vibrato.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    showClInfo();
    
    //  initialize variables to hold audio data
    std::string sInputFilePath,                 //!< file paths
                sOutputFilePath;

    static const int kBlockSize = 1024;

    clock_t time = 0;

    float **ppfAudioData = 0;

    CAudioFileIf *phInputAudioFile = 0;
    CAudioFileIf *phOutputAudioFile = 0;
    CAudioFileIf::FileSpec_t stFileSpec;
    
    //variables related to the vibrato
//    Vibrato *phVibrato = 0;
    float f_vfrequency;
    float f_vdepth;
    
    //check number of arguments
    if(argc < 2){
        cout << "No additional arguments. Running tests...";
        return 0;
    }
    else if (argc < 3){
        cout << "Missing frequency and depth arguments";
        return 0;
    }
    else if (argc < 4){
        cout << "Missing depth argument";
        return 0;
    }
    else{
        sInputFilePath = argv[1];
        sOutputFilePath = sInputFilePath + "_vibrato.wav";
        f_vfrequency = atof(argv[2]);
        f_vdepth = atof(argv[3]);
    }
    

    // open the input wave file
    CAudioFileIf::create(phInputAudioFile);
    phInputAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    if (!phInputAudioFile->isOpen())
    {
        cout << "Input Wave file open error!";
        CAudioFileIf::destroy(phInputAudioFile);
        return -1;
    }
    phInputAudioFile->getFileSpec(stFileSpec);
    
    // open the output wav file
    CAudioFileIf::create(phOutputAudioFile);
    phOutputAudioFile->openFile(sOutputFilePath, CAudioFileIf::kFileWrite, &stFileSpec);
    if (!phOutputAudioFile->isOpen())
    {
        cout << "Output Wave file open error!";
        CAudioFileIf::destroy(phOutputAudioFile);
        return -1;
    }
    
    //create vibrato object
    Vibrato hVibrato = Vibrato(stFileSpec.fSampleRateInHz, f_vdepth+1, stFileSpec.iNumChannels);
    hVibrato.setFrequency(f_vfrequency);
    hVibrato.setDepth(f_vdepth);
    
    // allocate memory
    ppfAudioData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfAudioData[i] = new float[kBlockSize];

    if (ppfAudioData == 0)
    {
        CAudioFileIf::destroy(phInputAudioFile);
        CAudioFileIf::destroy(phOutputAudioFile);
        return -1;
    }
    if (ppfAudioData[0] == 0)
    {
        CAudioFileIf::destroy(phInputAudioFile);
        CAudioFileIf::destroy(phOutputAudioFile);
        return -1;
    }
    
    //meat and potatoes code
    while (!phInputAudioFile->isEof())
       {
           // set block length variable
           long long iNumFrames = kBlockSize;

           // read data (iNumOfFrames might be updated!)
           phInputAudioFile->readData(ppfAudioData, iNumFrames);
           hVibrato.process(ppfAudioData, ppfAudioData, iNumFrames);
           phOutputAudioFile->writeData(ppfAudioData, iNumFrames);

           cout << "\r" << "reading and writing";
       }

       cout << "\nreading/writing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;
    
    // clean-up (close files and free memory)
    CAudioFileIf::destroy(phInputAudioFile);
    CAudioFileIf::destroy(phOutputAudioFile);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfAudioData[i];
    delete[] ppfAudioData;
    ppfAudioData = 0;

    // all done
    return 0;
    

}


void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}
