#include "MUSI6106Config.h"

#ifdef WITH_TESTS
#include "Vector.h"
#include "Vibrato.h"
#include "RingBuffer.h"

#include "gtest/gtest.h"


namespace vibrato_test {
    void CHECK_ARRAY_CLOSE(float* buffer1, float* buffer2, int iLength, float fTolerance)
    {
        for (int i = 0; i < iLength; i++)
        {
            EXPECT_NEAR(buffer1[i], buffer2[i], fTolerance);
        }
    }

//    void CHECK_ZERO_MODULATION(float* buffer1, float* buffer2, int iLength, int iDelayInSamples, float fTolerance)
//    {
//        ASSERT_GT(iLength - iDelayInSamples, 0);
//        for (int i = 0; i < (iLength - iDelayInSamples); i++){
//            EXPECT_NEAR(buffer1[i], buffer2[i + iDelayInSamples], fTolerance);
//        }
//    }
//
//    void CHECK_DC_INPUT(){}

    class TRingBuffer : public  testing::Test{
    protected:
        void SetUp();
        void TearDown();
        
    };

    class TVibrato : public  testing::Test{
    protected:
        void SetUp();
        void TearDown();
        
    };

    //Test 0: Int and float mod functions work as expected
    TEST_F(TRingBuffer, Mod){

    }
    //Test 1: Linear Interpolation works as expected
        //Test 1a: What happens when you try to wrap back around?
    TEST_F(TRingBuffer, Interpolate){
        
    }

    //Test 2: Read/Write negative index values or values greater than the buffer length
    TEST_F(TRingBuffer, CheckBounds){
        
    }

    //Test 3: Writing a fractional index value
    TEST_F(TRingBuffer, WriteFloatIndex){
        
    }

    //Test 4a: Writing when the buffer is full
    //Test 4b: Reading when the buffer is empty
    TEST_F(TRingBuffer, EmptyFull){
        
    }

    TEST_F(TVibrato, CheckZeroModulation){}
    TEST_F(TVibrato, CheckDCInput){}
    TEST_F(TVibrato, VaryingBlockSize){
        int blockSizes[] = {123, 234, 3456, 45678};
    }
    TEST_F(TVibrato, ZeroInput){}
    TEST_F(TVibrato, CustomTest){}


}

#endif //WITH_TESTS
