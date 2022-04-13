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
        float tail[irLength - 1];
        fastConv->flushBuffer(tail);
        for (int i = 0; i < irLength - 1; i++) {
            int index = i + inputLength - shift;
            EXPECT_EQ(tail[i], index < irLength ? impulseResponse[index] : 0);
        }
    }
}

#endif //WITH_TESTS
