
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string             sInputFilePath,                 //!< file paths
                            sOutputFilePath;

    static const int        kBlockSize = 1024;

    clock_t                 time = 0;

    float                   **ppfAudioData = 0;

    CAudioFileIf            *phAudioFile = 0;
    std::fstream            hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 3) {
        std::cerr << "Usage: ./MUSI6106Exec <input wave filename> <output text filename>" << std::endl;
        return 1;
    }
    sInputFilePath = argv[1];
    sOutputFilePath = argv[2];
 
    //////////////////////////////////////////////////////////////////////////////
    // open the input wave file
    CAudioFileIf::create(phAudioFile);
    phAudioFile->openFile(sInputFilePath, CAudioFileIf::FileIoType_t::kFileRead);
    // and determine number of channels
    CAudioFileIf::FileSpec_t fileSpec;
    phAudioFile->getFileSpec(fileSpec);
    const int numChannels = fileSpec.iNumChannels;

    //////////////////////////////////////////////////////////////////////////////
    // open the output text file
    hOutputFile.open(sOutputFilePath, std::fstream::out);
 
    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioData = new float *[numChannels];
    for (int ch = 0; ch < numChannels; ch++) {
        ppfAudioData[ch] = new float[kBlockSize];
    }
 
    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output text file (one column per channel)
    bool moreData = true;
    while (moreData) {
        long long numFrames = kBlockSize;
        phAudioFile->readData(ppfAudioData, numFrames);
        if (numFrames < kBlockSize) {
            moreData = false;
        }
        // Write one line per frame.
        for (int frame = 0; frame < numFrames; frame++) {
            // Write one column per channel.
            for (int ch = 0; ch < numChannels; ch++) {
                hOutputFile << ppfAudioData[ch][frame];
                if (ch < numChannels - 1) {
                    hOutputFile << " ";
                }
            }
            hOutputFile << std::endl;
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)
    hOutputFile.close();
    for (int ch = 0; ch < numChannels; ch++) {
        delete ppfAudioData[ch];
    }
    delete ppfAudioData;
    phAudioFile->closeFile();
    CAudioFileIf::destroy(phAudioFile);

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
