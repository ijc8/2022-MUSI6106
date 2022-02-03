
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>

#include "MUSI6106Config.h"

#include "RingBuffer.h"

using std::cout;
using std::endl;

void showClInfo() {
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout << endl;
}

// Helper for testing.
int expectIdx;

void startTest() {
    expectIdx = 0;
}

void expect(bool b) {
    if (!b) throw expectIdx;
    expectIdx++;
}

// Test that ring buffer is a ring (wraps after more than `length` elements have entered).
void testWrapping() {
    const int length = 17;
    CRingBuffer<float> ringBuffer(length);

    for (int i = 0; i < 5; i++) {
        ringBuffer.putPostInc(1.F*i);
    }

    for (int i = 5; i < length + 13; i++) {
        expect(ringBuffer.getNumValuesInBuffer() == 5); // should be five
        expect(ringBuffer.getPostInc() == i - 5); // should be i-5
        ringBuffer.putPostInc(1.F*i);
    }
}

// Test for generality; does this handle a more complex, non-numeric type correctly?
void testString() {
    CRingBuffer<std::string> stringBuffer(3);
    stringBuffer.putPostInc("hello");
    stringBuffer.putPostInc("world");
    stringBuffer.putPostInc("!!!");
    expect(stringBuffer.getPostInc() == "hello");
    expect(stringBuffer.getPostInc() == "world");
    expect(stringBuffer.getPostInc() == "!!!");
}

// Basic test of all API functions.
void testAPI() {
    const int length = 3;
    CRingBuffer<int> ringBuffer(length);

    expect(ringBuffer.getLength() == length);

    ringBuffer.put(3);
    expect(ringBuffer.get() == 3);

    ringBuffer.setWriteIdx(1);
    expect(ringBuffer.getWriteIdx() == 1);

    ringBuffer.putPostInc(17);
    expect(ringBuffer.getWriteIdx() == 2);

    expect(ringBuffer.getReadIdx() == 0);
    expect(ringBuffer.get(1) == 17);
    expect(ringBuffer.getPostInc() == 3);
    expect(ringBuffer.getReadIdx() == 1);

    expect(ringBuffer.getNumValuesInBuffer() == 1);
    ringBuffer.putPostInc(42);
    expect(ringBuffer.getNumValuesInBuffer() == 2);

    expect(ringBuffer.getWriteIdx() == 0);

    // Should be unchanged.
    expect(ringBuffer.getLength() == length);
}

// Test state after initialization and reset.
void testReset() {
    CRingBuffer<float> ringBuffer(512);

    // Check initial state.
    expect(ringBuffer.getReadIdx() == 0);
    expect(ringBuffer.getWriteIdx() == 0);
    for (int i = 0; i < ringBuffer.getLength(); i++) {
        expect(ringBuffer.get(i) == 0.0f);
    }

    // Fill ring buffer, mess with indices.
    const float fill = 123.456f;
    for (int i = 0; i < ringBuffer.getLength(); i++) {
        ringBuffer.putPostInc(fill);
        expect(ringBuffer.get(i) == fill);
    }

    ringBuffer.setWriteIdx(17);
    ringBuffer.setReadIdx(42);

    // Check state after reset.
    ringBuffer.reset();
    expect(ringBuffer.getReadIdx() == 0);
    expect(ringBuffer.getWriteIdx() == 0);
    for (int i = 0; i < ringBuffer.getLength(); i++) {
        expect(ringBuffer.get(i) == 0.0f);
    }
}

// Test inputs to setWriteIdx/setReadIdx outside of bounds [0, length).
void testWeirdInputs() {
    CRingBuffer<float> ringBuffer(5);

    ringBuffer.setWriteIdx(5);
    expect(ringBuffer.getWriteIdx() == 0);
    ringBuffer.setWriteIdx(17);
    expect(ringBuffer.getWriteIdx() == 2);
    ringBuffer.setWriteIdx(-2);
    expect(ringBuffer.getWriteIdx() == 3);

    ringBuffer.setReadIdx(5);
    expect(ringBuffer.getReadIdx() == 0);
    ringBuffer.setReadIdx(17);
    expect(ringBuffer.getReadIdx() == 2);
    ringBuffer.setReadIdx(-2);
    expect(ringBuffer.getReadIdx() == 3);
}

// Test as a delay using a random signal.
void testSignal() {
    const int signalLength = 1024;
    int signal[signalLength];
    CRingBuffer<int> ringBuffer(256);

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
            expect(ringBuffer.getPostInc() == signal[i - delay]);
        }
    }
}

int main(int argc, char* argv[]) {
    showClInfo();

    // Run tests.
    auto tests = {testWrapping, testString, testAPI, testReset, testWeirdInputs, testSignal};
    int passed = 0;
    for (int i = 0; i < tests.size(); i++) {
        try {
            startTest();
            tests.begin()[i]();
            cout << "Test " << i << " passed ✓" << endl;
            passed++;
        } catch (int &expectIdx) {
            cout << "Test " << i << " failed ✗ (at expect " << expectIdx << ")" << endl;
        }
    }
    cout << "Tests passed: " << passed << "/" << tests.size() << endl;

    return 0;
}
