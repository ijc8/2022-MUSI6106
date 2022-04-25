#include "MUSI6106Config.h"

#ifdef WITH_TESTS
#include "Synthesis.h"

#include "Vector.h"
#include "FastConv.h"

#include "gtest/gtest.h"


namespace fastconv_test {
    void CHECK_ARRAY_CLOSE(float* buffer1, float* buffer2, int iLength, float fTolerance) {
        for (int i = 0; i < iLength; i++) {
            EXPECT_NEAR(buffer1[i], buffer2[i], fTolerance);
        }
    }

    class FastConv: public testing::Test {
    protected:
        void SetUp() override {
            fastConv = new CFastConv;
        }

        virtual void TearDown() {
            delete fastConv;
        }

        CFastConv *fastConv = 0;
        const float epsilon = 1e-5;
    };

    TEST_F(FastConv, Identity) {
        const int irLength = 51;
        float impulseResponse[irLength];
        // Generate random impulse response (samples from -1 to 1).
        for (int i = 0; i < irLength; i++) {
            impulseResponse[i] = (float)rand() / RAND_MAX * 2 - 1;
        }

        fastConv->init(impulseResponse, irLength, 0, CFastConv::kTimeDomain);
        const int inputLength = 10;
        int shift = 3;
        float input[inputLength] = {0};
        input[shift] = 1;
        float output[inputLength];
        fastConv->process(output, input, inputLength);
        for (int i = 0; i < inputLength; i++) {
            EXPECT_EQ(output[i], i < shift ? 0 : impulseResponse[i - shift]);
        }
    }

    TEST_F(FastConv, FlushBuffer) {
        const int irLength = 51;
        float impulseResponse[irLength];
        // Generate random impulse response (samples from -1 to 1).
        for (int i = 0; i < irLength; i++) {
            impulseResponse[i] = (float)rand() / RAND_MAX * 2 - 1;
        }

        fastConv->init(impulseResponse, irLength, 0, CFastConv::kTimeDomain);
        const int inputLength = 10;
        int shift = 3;
        float input[inputLength] = {0};
        input[shift] = 1;
        float output[inputLength];
        fastConv->process(output, input, inputLength);

        // Check tail correctness.
        int tailLength = fastConv->getTailLength();
        std::vector<float> tail(tailLength);
        fastConv->flushBuffer(tail.data());
        for (int i = 0; i < tailLength; i++) {
            int index = i + inputLength - shift;
            EXPECT_EQ(tail[i], index < irLength ? impulseResponse[index] : 0);
        }
    }

    TEST_F(FastConv, BlockSize) {
        const int irLength = 10;
        float impulseResponse[irLength] = {0};
        int shift = 3;
        impulseResponse[shift] = 1;

        const int inputLength = 10000;
        float input[inputLength];
        // Generate random input signal (samples from -1 to 1).
        for (int i = 0; i < inputLength; i++) {
            input[i] = (float)rand() / RAND_MAX * 2 - 1;
        }
        float output[inputLength];

        fastConv->init(impulseResponse, irLength, 0, CFastConv::kTimeDomain);

        // NOTE: These block sizes sum to `inputLength`.
        int i = 0;
        for (int blockSize : {1, 13, 1023, 2048, 1, 17, 5000, 1897}) {
            fastConv->process(&output[i], &input[i], blockSize);
            i += blockSize;
        }

        for (int i = 0; i < inputLength; i++) {
            EXPECT_EQ(output[i], i < shift ? 0 : input[i - shift]);
        }
    }

    TEST_F(FastConv, IdentityFreq) {
        const int irLength = 51;
        float impulseResponse[irLength];
        // Generate random impulse response (samples from -1 to 1).
        for (int i = 0; i < irLength; i++) {
            impulseResponse[i] = (float)rand() / RAND_MAX * 2 - 1;
        }

        int blockLength = 4;
        fastConv->init(impulseResponse, irLength, blockLength, CFastConv::kFreqDomain);
        const int inputLength = 10;
        int shift = 3;
        float input[inputLength] = {0};
        input[shift] = 1;
        float output[inputLength];
        fastConv->process(output, input, inputLength);
        for (int i = 0; i < inputLength; i++) {
            EXPECT_NEAR(output[i], i < (shift + blockLength) ? 0 : impulseResponse[i - (shift + blockLength)], epsilon);
        }
    }

    TEST_F(FastConv, FlushBufferFreq) {
        const int irLength = 51;
        float impulseResponse[irLength];
        // Generate random impulse response (samples from -1 to 1).
        for (int i = 0; i < irLength; i++) {
            impulseResponse[i] = (float)rand() / RAND_MAX * 2 - 1;
        }

        int blockLength = 8;
        fastConv->init(impulseResponse, irLength, blockLength, CFastConv::kFreqDomain);
        const int inputLength = 10;
        int shift = 3;
        float input[inputLength] = {0};
        input[shift] = 1;
        float output[inputLength];
        fastConv->process(output, input, inputLength);

        // Check tail correctness.
        int tailLength = fastConv->getTailLength();
        std::vector<float> tail(tailLength);
        fastConv->flushBuffer(tail.data());
        for (int i = 0; i < tailLength; i++) {
            int index = i + inputLength - (shift + blockLength);
            EXPECT_NEAR(tail[i], index >= 0 && index < irLength ? impulseResponse[index] : 0, epsilon);
        }
    }

    TEST_F(FastConv, BlockSizeFreq) {
        const int irLength = 10;
        float impulseResponse[irLength] = {0};
        int shift = 3;
        impulseResponse[shift] = 1;

        const int inputLength = 10000;
        float input[inputLength];
        // Generate random input signal (samples from -1 to 1).
        for (int i = 0; i < inputLength; i++) {
            input[i] = (float)rand() / RAND_MAX * 2 - 1;
        }
        float output[inputLength];

        int blockLength = 2048;
        fastConv->init(impulseResponse, irLength, blockLength, CFastConv::kFreqDomain);

        // NOTE: These block sizes sum to `inputLength`.
        int i = 0;
        for (int blockSize : {1, 13, 1023, 2048, 1, 17, 5000, 1897}) {
            fastConv->process(&output[i], &input[i], blockSize);
            i += blockSize;
        }

        for (int i = 0; i < inputLength; i++) {
            EXPECT_NEAR(output[i], i < shift + blockLength ? 0 : input[i - (shift + blockLength)], epsilon);
        }
    }

    // Extra test: confirm that time and frequency domain results are (nearly) equal.
    TEST_F(FastConv, Consistent) {
        const int irLength = 51;
        float impulseResponse[irLength];
        // Generate random impulse response (samples from -1 to 1).
        for (int i = 0; i < irLength; i++) {
            impulseResponse[i] = (float)rand() / RAND_MAX * 2 - 1;
        }

        const int inputLength = 10000;
        float input[inputLength];
        // Generate random input signal (samples from -1 to 1).
        for (int i = 0; i < inputLength; i++) {
            input[i] = (float)rand() / RAND_MAX * 2 - 1;
        }
        float freqOutput[inputLength];
        float timeOutput[inputLength];

        CFastConv slowConv;
        int blockLength = 8;
        fastConv->init(impulseResponse, irLength, blockLength, CFastConv::kFreqDomain);
        slowConv.init(impulseResponse, irLength, 0, CFastConv::kTimeDomain);

        // NOTE: These block sizes sum to `inputLength`.
        int i = 0;
        for (int blockSize : {1, 13, 1023, 2048, 1, 17, 5000, 1897}) {
            fastConv->process(&freqOutput[i], &input[i], blockSize);
            slowConv.process(&timeOutput[i], &input[i], blockSize);
            i += blockSize;
        }

        for (int i = 0; i < inputLength; i++) {
            EXPECT_NEAR(freqOutput[i], i < blockLength ? 0 : timeOutput[i - blockLength], epsilon);
        }
    }

    // Extra test: check error conditions.
    TEST_F(FastConv, Errors) {
        const int irLength = 10;
        float ir[irLength] = {0};
        EXPECT_EQ(fastConv->init(ir, irLength), Error_t::kNoError);
        EXPECT_EQ(fastConv->reset(), Error_t::kNoError);
        // Try non-power-of-2 FFT size.
        EXPECT_EQ(fastConv->init(ir, irLength, 13), Error_t::kFunctionInvalidArgsError);
        // Try invalid convolution type.
        EXPECT_EQ(fastConv->init(ir, irLength, 2048, CFastConv::kNumConvCompModes), Error_t::kFunctionInvalidArgsError);
        // Try performing operations without initialization.
        float input[1], output[1];
        EXPECT_EQ(fastConv->process(output, input, 1), Error_t::kNotInitializedError);
        EXPECT_EQ(fastConv->flushBuffer(output), Error_t::kNotInitializedError);
    }
}

#endif //WITH_TESTS
