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

    class Vibrato : public  testing::Test{
    protected:
        void SetUp();
        void TearDown();
        
    };

    TEST_F(Vibrato, RingBufferTests){
        //Test 0: Int and float mod functions work as expected
        //Test 1: Linear Interpolation works as expected
            //Test 1a: What happens when you try to wrap back around?
        //Test 2: Read/Write negative index values or values greater than the buffer length
        //Test 3: Writing a fractional value
        //Test 4a: Writing when the buffer is full
        //Test 4b: Reading when the buffer is empty
        
        
    }
    TEST_F(Vibrato, CheckZeroModulation){}
    TEST_F(Vibrato, CheckDCInput){}
    TEST_F(Vibrato, VaryingBlockSize){}
    TEST_F(Vibrato, ZeroInput){}
    TEST_F(Vibrato, CustomTest){}


}

#endif //WITH_TESTS
