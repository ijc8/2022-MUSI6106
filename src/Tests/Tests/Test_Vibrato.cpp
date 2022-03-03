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

    // Test for generality; does this handle a more complex, non-numeric type correctly?
    TEST_F(RingBufferTest, String) {
        RingBuffer<std::string> stringBuffer(3);
        stringBuffer.putPostInc("hello");
        stringBuffer.putPostInc("world");
        stringBuffer.putPostInc("!!!");
        EXPECT_EQ(stringBuffer.getPostInc(), "hello");
        EXPECT_EQ(stringBuffer.getPostInc(), "world");
        EXPECT_EQ(stringBuffer.getPostInc(), "!!!");
    }

    // Basic test of all API functions.
    TEST_F(RingBufferTest, API) {
        const int length = 3;
        RingBuffer<int> ringBuffer(length);

        EXPECT_EQ(ringBuffer.getLength(), length);

        ringBuffer.put(3);
        EXPECT_EQ(ringBuffer.get(), 3);

        ringBuffer.setWriteIdx(1);
        EXPECT_EQ(ringBuffer.getWriteIdx(), 1);

        ringBuffer.putPostInc(17);
        EXPECT_EQ(ringBuffer.getWriteIdx(), 2);

        EXPECT_EQ(ringBuffer.getReadIdx(), 0);
        EXPECT_EQ(ringBuffer.get(1), 17);
        EXPECT_EQ(ringBuffer.getPostInc(), 3);
        EXPECT_EQ(ringBuffer.getReadIdx(), 1);

        EXPECT_EQ(ringBuffer.getNumValuesInBuffer(), 1);
        ringBuffer.putPostInc(42);
        EXPECT_EQ(ringBuffer.getNumValuesInBuffer(), 2);

        EXPECT_EQ(ringBuffer.getWriteIdx(), 0);

        // Should be unchanged.
        EXPECT_EQ(ringBuffer.getLength(), length);
    }

    // Test state after initialization and reset.
    TEST_F(RingBufferTest, Reset) {
        RingBuffer<float> ringBuffer(512);

        // Check initial state.
        EXPECT_EQ(ringBuffer.getReadIdx(), 0);
        EXPECT_EQ(ringBuffer.getWriteIdx(), 0);
        for (int i = 0; i < ringBuffer.getLength(); i++) {
            EXPECT_EQ(ringBuffer.get(i), 0.0f);
        }

        // Fill ring buffer, mess with indices.
        const float fill = 123.456f;
        for (int i = 0; i < ringBuffer.getLength(); i++) {
            ringBuffer.putPostInc(fill);
            EXPECT_EQ(ringBuffer.get(i), fill);
        }

        ringBuffer.setWriteIdx(17);
        ringBuffer.setReadIdx(42);

        // Check state after reset.
        ringBuffer.reset();
        EXPECT_EQ(ringBuffer.getReadIdx(), 0);
        EXPECT_EQ(ringBuffer.getWriteIdx(), 0);
        for (int i = 0; i < ringBuffer.getLength(); i++) {
            EXPECT_EQ(ringBuffer.get(i), 0.0f);
        }
    }

    // Test inputs to setWriteIdx/setReadIdx outside of bounds [0, length).
    TEST_F(RingBufferTest, Bounds) {
        RingBuffer<float> ringBuffer(5);

        ringBuffer.setWriteIdx(5);
        EXPECT_EQ(ringBuffer.getWriteIdx(), 0);
        ringBuffer.setWriteIdx(17);
        EXPECT_EQ(ringBuffer.getWriteIdx(), 2);
        ringBuffer.setWriteIdx(-2);
        EXPECT_EQ(ringBuffer.getWriteIdx(), 3);

        ringBuffer.setReadIdx(5);
        EXPECT_EQ(ringBuffer.getReadIdx(), 0);
        ringBuffer.setReadIdx(17);
        EXPECT_EQ(ringBuffer.getReadIdx(), 2);
        ringBuffer.setReadIdx(-2);
        EXPECT_EQ(ringBuffer.getReadIdx(), 3);
    }

    // Test as a delay using a random signal.
    TEST_F(RingBufferTest, Signal) {
        const int signalLength = 1024;
        int signal[signalLength];
        RingBuffer<int> ringBuffer(256);

        // Generate random signal.
        srand(time(NULL));
        for (int i = 0; i < signalLength; i++) {
            signal[i] = rand();
        }

        // Feed signal to ring buffer.
        // Wait for `delay` to start taking values out,
        // and then ensure the signal is delayed as expected.
        int delay = 100;
        for (int i = 0; i < ringBuffer.getLength() * 4; i++) {
            ringBuffer.putPostInc(signal[i]);
            if (i >= delay) {
                EXPECT_EQ(ringBuffer.getPostInc(), signal[i - delay]);
            }
        }
    }

    // Test fractional offset.
    TEST_F(RingBufferTest, Fraction) {
        RingBuffer<float> ringBuffer(3);
        ringBuffer.putPostInc(17);
        ringBuffer.putPostInc(40);
        ringBuffer.putPostInc(-5);
        EXPECT_EQ(ringBuffer.get(0.5f), (17 + 40)/2.0);
        EXPECT_EQ(ringBuffer.get(1.5f), (40 - 5)/2.0);
        EXPECT_EQ(ringBuffer.get(-0.5f), (17 - 5)/2.0);
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
