#include "MUSI6106Config.h"

#ifdef WITH_TESTS
#include "Vector.h"
#include "Vibrato.h"
#include "RingBuffer.h"

#include "gtest/gtest.h"


namespace vibrato_test {
    void CHECK_ARRAY_CLOSE(float* buffer1, float* buffer2, int iLength, float fTolerance) {
        for (int i = 0; i < iLength; i++) {
            EXPECT_NEAR(buffer1[i], buffer2[i], fTolerance);
        }
    }

    void CHECK_ZERO_MODULATION(float* buffer1, float* buffer2, int iLength, int iDelayInSamples, float fTolerance) {
        ASSERT_GT(iLength - iDelayInSamples, 0);
        for (int i = 0; i < (iLength - iDelayInSamples); i++) {
            EXPECT_NEAR(buffer1[i], buffer2[i + iDelayInSamples], fTolerance);
        }
    }

    void CHECK_DC_INPUT(){}

    class RingBuffer: public testing::Test {
    protected:
        void SetUp() override {};
        virtual void TearDown() {};
        
    };

    // Test 0: Int and float mod functions work as expected
    TEST_F(RingBuffer, Mod) {
        EXPECT_TRUE(false);
    }
    // Test 1: Linear Interpolation works as expected
    // Test 1a: What happens when you try to wrap back around?
    TEST_F(RingBuffer, Interpolate) {
        EXPECT_TRUE(false);
    }

    // Test 2: Read/Write negative index values or values greater than the buffer length
    TEST_F(RingBuffer, CheckBounds) {
        EXPECT_TRUE(false);
    }

    // Test 3: Writing a fractional index value
    TEST_F(RingBuffer, WriteFloatIndex) {
        EXPECT_TRUE(false);
    }

    // Test 4a: Writing when the buffer is full
    // Test 4b: Reading when the buffer is empty
    TEST_F(RingBuffer, EmptyFull) {
        EXPECT_TRUE(false);
    }

    class Vibrato: public testing::Test {
    protected:
        void SetUp() override {};
        virtual void TearDown() {};
    };

    TEST_F(Vibrato, CheckZeroModulation) {
        EXPECT_TRUE(false);
    }

    TEST_F(Vibrato, CheckDCInput) {
        EXPECT_TRUE(false);
    }

    TEST_F(Vibrato, VaryingBlockSize) {
        int blockSizes[] = {123, 234, 3456, 45678};
        EXPECT_TRUE(false);
    }

    TEST_F(Vibrato, ZeroInput) {
        EXPECT_TRUE(false);
    }

    TEST_F(Vibrato, CustomTest) {
        EXPECT_TRUE(false);
    }
}

#endif //WITH_TESTS
