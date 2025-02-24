
#define _USE_MATH_DEFINES
#include <cmath>

#include "Util.h"
#include "Vector.h"
#include "Fft.h"

#include "rvfft.h"

const float CFft::m_Pi  = static_cast<float>(M_PI);
const float CFft::m_Pi2 = static_cast<float>(M_PI_2);

CFft::CFft() :
    m_pfProcessBuff(0),
    m_pfWindowBuff(0),
    m_iDataLength(0),
    m_iFftLength(0),
    m_ePrePostWindowOpt(kNoWindow),
    m_bIsInitialized(false)
{
    resetInstance ();
}

Error_t CFft::createInstance( CFft*& pCFft )
{
    pCFft = new CFft ();

    if (!pCFft)
        return Error_t::kMemError;

    return Error_t::kNoError;
}

Error_t CFft::destroyInstance( CFft*& pCFft )
{
    if (!pCFft)
        return Error_t::kNoError;

    delete pCFft;
    pCFft   = 0;

    return Error_t::kNoError;
}

Error_t CFft::initInstance( int iBlockLength, int iZeroPadFactor, WindowFunction_t eWindow /*= kWindowHann*/, Windowing_t eWindowing /*= kPreWindow*/ )
{
    Error_t  rErr = Error_t::kNoError;

    // sanity check
    if (!CUtil::isPowOf2(iBlockLength) || iZeroPadFactor <= 0 || !CUtil::isPowOf2(iBlockLength*iZeroPadFactor))
        return Error_t::kFunctionInvalidArgsError;

    // clean up
    resetInstance();

    m_iDataLength   = iBlockLength;
    m_iFftLength    = iBlockLength * iZeroPadFactor;

    m_ePrePostWindowOpt = eWindowing;

    rErr = allocMemory ();
    if (rErr != Error_t::kNoError)
        return rErr;

    rErr = computeWindow (eWindow);

    m_bIsInitialized    = true;

    return rErr;
}

Error_t CFft::resetInstance()
{
    freeMemory();

    m_iDataLength       = 0;
    m_iFftLength        = 0;
    m_ePrePostWindowOpt = kNoWindow;
    
    m_bIsInitialized    = false;

    return Error_t::kNoError;

}

Error_t CFft::overrideWindow( const float *pfNewWindow )
{
    if (!m_bIsInitialized)
        return Error_t::kNotInitializedError;
    if (!pfNewWindow)
        return Error_t::kFunctionInvalidArgsError;

    CVectorFloat::copy(m_pfWindowBuff, pfNewWindow, m_iDataLength);

    return Error_t::kNoError;
}

Error_t CFft::getWindow( float *pfWindow ) const
{
    if (!m_bIsInitialized)
        return Error_t::kNotInitializedError;
    if (!pfWindow)
        return Error_t::kFunctionInvalidArgsError;

    CVectorFloat::copy(pfWindow, m_pfWindowBuff, m_iDataLength);

    return Error_t::kNoError;
}

Error_t CFft::doFft( complex_t *pfSpectrum, const float *pfInput )
{
    if (!m_bIsInitialized)
        return Error_t::kNotInitializedError;
    if (!pfInput || !pfSpectrum)
        return Error_t::kFunctionInvalidArgsError;

    // copy data to internal buffer
    CVectorFloat::copy(m_pfProcessBuff, pfInput, m_iDataLength);
    CVectorFloat::setZero(&m_pfProcessBuff[m_iDataLength], m_iFftLength-m_iDataLength);

    // apply window function
    if (m_ePrePostWindowOpt & kPreWindow)
        CVectorFloat::mul_I(m_pfProcessBuff, m_pfWindowBuff, m_iDataLength);

    // compute fft
    LaszloFft::realfft_split(m_pfProcessBuff, m_iFftLength);

    // copy data to output buffer
    CVectorFloat::copy(pfSpectrum, m_pfProcessBuff, m_iFftLength);

    return Error_t::kNoError;
}

Error_t CFft::doInvFft( float *pfOutput, const complex_t *pfSpectrum )
{
    if (!m_bIsInitialized)
        return Error_t::kNotInitializedError;

    // copy data to internal buffer
    CVectorFloat::copy(m_pfProcessBuff, pfSpectrum, m_iFftLength);
    
    // compute ifft
    LaszloFft::irealfft_split(m_pfProcessBuff, m_iFftLength);

    // apply window function
    if (m_ePrePostWindowOpt & kPostWindow)
        CVectorFloat::mul_I(m_pfProcessBuff, m_pfWindowBuff, m_iDataLength);

    // copy data to output buffer
    CVectorFloat::copy(pfOutput, m_pfProcessBuff, m_iFftLength);

    return Error_t::kNoError;
}

Error_t CFft::getMagnitude( float *pfMag, const complex_t *pfSpectrum ) const
{
    if (!m_bIsInitialized)
        return Error_t::kNotInitializedError;

    // re(0),re(1),re(2),...,re(size/2),im(size/2-1),...,im(1)
    int iNyq        = m_iFftLength>>1;

    // no imaginary part at these bins
    pfMag[0]        = std::abs(pfSpectrum[0]);
    pfMag[iNyq]     = std::abs(pfSpectrum[iNyq]);

    for (int i = 1; i < iNyq; i++)
    {
        int iImagIdx    = m_iFftLength - i;
        pfMag[i]        = sqrtf(pfSpectrum[i]*pfSpectrum[i] + pfSpectrum[iImagIdx]*pfSpectrum[iImagIdx]);
    }
    return Error_t::kNoError;
}

Error_t CFft::getPhase( float *pfPhase, const complex_t *pfSpectrum ) const
{
    if (!m_bIsInitialized)
        return Error_t::kNotInitializedError;

    // re(0),re(1),re(2),...,re(size/2),im(size/2-1),...,im(1)
    int iNyq        = m_iFftLength>>1;

    pfPhase[0]      = m_Pi;
    pfPhase[iNyq]   = m_Pi;
    
    for (int i = 1; i < iNyq; i++)
    {
        int iImagIdx    = m_iFftLength - i;
        if (pfSpectrum[i] == .0F && pfSpectrum[iImagIdx] != .0F)
            pfPhase[i]   = m_Pi2;
        else
            pfPhase[i]   = atan2f (pfSpectrum[iImagIdx], pfSpectrum[i]);
    }
    return Error_t::kNoError;
}

Error_t CFft::splitRealImag( float *pfReal, float *pfImag, const complex_t *pfSpectrum ) const
{
    if (!m_bIsInitialized)
        return Error_t::kNotInitializedError;

    // re(0),re(1),re(2),...,re(size/2),im(size/2-1),...,im(1)
    int iNyq        = m_iFftLength>>1;

    CVectorFloat::copy(pfReal, pfSpectrum, iNyq+1);

    pfImag[0]       = 0;
    for (int i = 1, iImag = m_iFftLength-1; i < iNyq; i++, iImag--)
    {
        pfImag[i]   = pfSpectrum[iImag];
    }

    return Error_t::kNoError;
}

Error_t CFft::mergeRealImag( complex_t *pfSpectrum, const float *pfReal, const float *pfImag ) const
{
    if (!m_bIsInitialized)
        return Error_t::kNotInitializedError;

    // re(0),re(1),re(2),...,re(size/2),im(size/2-1),...,im(1)
    int iNyq        = m_iFftLength>>1;

    CVectorFloat::copy(pfSpectrum, pfReal, iNyq+1);

    for (int i = 1, iImag = m_iFftLength-1; i < iNyq-1; i++, iImag--)
    {
        pfSpectrum[iImag]   = pfImag[i];
    }

    return Error_t::kNoError;
}

float CFft::freq2bin( float fFreqInHz, float fSampleRateInHz ) const
{
    return fFreqInHz / fSampleRateInHz * m_iFftLength;
}

float CFft::bin2freq( int iBinIdx, float fSampleRateInHz ) const
{
    return iBinIdx * fSampleRateInHz / m_iFftLength;
}

Error_t CFft::allocMemory()
{

    m_pfProcessBuff = new float [m_iFftLength];
    m_pfWindowBuff  = new float [m_iDataLength];

    if (!m_pfProcessBuff || !m_pfWindowBuff)
        return Error_t::kMemError;
    else
    {
        return Error_t::kNoError;
    }
}

Error_t CFft::freeMemory()
{
    delete [] m_pfProcessBuff;
    delete [] m_pfWindowBuff;

    m_pfProcessBuff = 0;
    m_pfWindowBuff  = 0;

    return Error_t::kNoError;
}

Error_t CFft::computeWindow( WindowFunction_t eWindow )
{
    int i;

    // note that these windows are periodic, not symmetric
    switch (eWindow)
    {
    case kWindowSine:
        {
            for (i = 0; i < m_iDataLength; i++)
            {
                m_pfWindowBuff[i]   = sinf((i * m_Pi) / (m_iDataLength+1));
            }
            break;
        }
    case kWindowHann:
        {
            for (i = 0; i < m_iDataLength; i++)
            {
                m_pfWindowBuff[i]   = .5F * (1.F - cosf((i * 2.F*m_Pi) / (m_iDataLength+1)));
            }
            break;
        }
    case kWindowHamming:
        {
            for (i = 0; i < m_iDataLength; i++)
            {
                m_pfWindowBuff[i]   = .54F - .46F * cosf((i * 2.F*m_Pi) / (m_iDataLength+1));
            }
            break;
        }
    }

    return Error_t::kNoError;
}

int CFft::getLength( Length_t eLengthIdx )
{
    switch (eLengthIdx)
    {
    case kLengthFft:
        return m_iFftLength;
    case kLengthData:
        return m_iDataLength;
    case kLengthMagnitude:
    case kLengthPhase:
        return m_iFftLength/2+1;
    }
    return 0;
}
