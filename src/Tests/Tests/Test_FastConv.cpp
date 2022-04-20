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
    };

    TEST_F(FastConv, Identity) {
        int irLength = 51;
        float impulseResponse[irLength];
        // Generate random impulse response (samples from -1 to 1).
        for (int i = 0; i < irLength; i++) {
            impulseResponse[i] = (float)rand() / RAND_MAX * 2 - 1;
        }

        fastConv->init(impulseResponse, irLength, 0, CFastConv::kTimeDomain);
        int inputLength = 10;
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
        int irLength = 51;
        float impulseResponse[irLength];
        // Generate random impulse response (samples from -1 to 1).
        for (int i = 0; i < irLength; i++) {
            impulseResponse[i] = (float)rand() / RAND_MAX * 2 - 1;
        }

        fastConv->init(impulseResponse, irLength, 0, CFastConv::kTimeDomain);
        int inputLength = 10;
        int shift = 3;
        float input[inputLength] = {0};
        input[shift] = 1;
        float output[inputLength];
        fastConv->process(output, input, inputLength);

        // Check tail correctness.
        float tail[irLength - 1];
        fastConv->flushBuffer(tail);
        for (int i = 0; i < irLength - 1; i++) {
            int index = i + inputLength - shift;
            EXPECT_EQ(tail[i], index < irLength ? impulseResponse[index] : 0);
        }
    }

    TEST_F(FastConv, BlockSize) {
        int irLength = 10;
        float impulseResponse[irLength] = {0};
        int shift = 3;
        impulseResponse[shift] = 1;

        int inputLength = 10000;
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
        int irLength = 51;
        float impulseResponse[irLength];
        // Generate random impulse response (samples from -1 to 1).
        for (int i = 0; i < irLength; i++) {
            impulseResponse[i] = (float)rand() / RAND_MAX * 2 - 1;
        }

        int blockLength = 4;
        fastConv->init(impulseResponse, irLength, blockLength, CFastConv::kFreqDomain);
        int inputLength = 10;
        int shift = 3;
        float input[inputLength] = {0};
        input[shift] = 1;
        float output[inputLength];
        fastConv->process(output, input, inputLength);
        for (int i = 0; i < inputLength; i++) {
            EXPECT_EQ(output[i], i < (shift + blockLength) ? 0 : impulseResponse[i - (shift + blockLength)]);
        }
    }

    TEST_F(FastConv, FlushBufferFreq) {
        int irLength = 51;
        float impulseResponse[irLength];
        // Generate random impulse response (samples from -1 to 1).
        for (int i = 0; i < irLength; i++) {
            impulseResponse[i] = (float)rand() / RAND_MAX * 2 - 1;
        }

        int blockLength = 8;
        fastConv->init(impulseResponse, irLength, blockLength, CFastConv::kFreqDomain);
        int inputLength = 10;
        int shift = 3;
        float input[inputLength] = {0};
        input[shift] = 1;
        float output[inputLength];
        fastConv->process(output, input, inputLength);

        // Check tail correctness.
        int tailLength = irLength - 1 + blockLength;
        float tail[tailLength];
        fastConv->flushBuffer(tail);
        for (int i = 0; i < tailLength; i++) {
            int index = i + inputLength - (shift + blockLength);
            EXPECT_EQ(tail[i], index >= 0 && index < irLength ? impulseResponse[index] : 0);
        }
    }

    TEST_F(FastConv, BlockSizeFreq) {
        int irLength = 10;
        float impulseResponse[irLength] = {0};
        int shift = 3;
        impulseResponse[shift] = 1;

        int inputLength = 10000;
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
            EXPECT_EQ(output[i], i < shift + blockLength ? 0 : input[i - (shift + blockLength)]);
        }
    }
}

#endif //WITH_TESTS
