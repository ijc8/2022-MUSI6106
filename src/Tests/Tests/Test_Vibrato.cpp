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

    class RingBufferTest: public testing::Test {
    protected:
        void SetUp() override {};
        virtual void TearDown() {};
        
    };

    // Test that ring buffer is a ring (wraps after more than `length` elements have entered).
    TEST_F(RingBufferTest, Wrapping) {
        const int length = 17;
        RingBuffer<float> ringBuffer(length);

        for (int i = 0; i < 5; i++) {
            ringBuffer.putPostInc(1.F*i);
        }

        for (int i = 5; i < length + 13; i++) {
            EXPECT_EQ(ringBuffer.getNumValuesInBuffer(), 5);
            EXPECT_EQ(ringBuffer.getPostInc(), i - 5);
            ringBuffer.putPostInc(1.F*i);
        }
    }
    // Test 1: Linear Interpolation works as expected
    // Test 1a: What happens when you try to wrap back around?
    TEST_F(RingBufferTest, Interpolate) {
        EXPECT_TRUE(false);
    }

    // Test 2: Read/Write negative index values or values greater than the buffer length
    TEST_F(RingBufferTest, CheckBounds) {
        EXPECT_TRUE(false);
    }

    // Test 3: Writing a fractional index value
    TEST_F(RingBufferTest, WriteFloatIndex) {
        EXPECT_TRUE(false);
    }

    // Test 4a: Writing when the buffer is full
    // Test 4b: Reading when the buffer is empty
    TEST_F(RingBufferTest, EmptyFull) {
        EXPECT_TRUE(false);
    }

    class VibratoTest: public testing::Test {
    protected:
        void SetUp() override {};
        virtual void TearDown() {};
    };

    TEST_F(VibratoTest, CheckZeroModulation) {
        EXPECT_TRUE(false);
    }

    TEST_F(VibratoTest, CheckDCInput) {
        EXPECT_TRUE(false);
    }

    TEST_F(VibratoTest, VaryingBlockSize) {
        int blockSizes[] = {123, 234, 3456, 45678};
        EXPECT_TRUE(false);
    }

    TEST_F(VibratoTest, ZeroInput) {
        EXPECT_TRUE(false);
    }

    TEST_F(VibratoTest, CustomTest) {
        EXPECT_TRUE(false);
    }
}

#endif //WITH_TESTS
