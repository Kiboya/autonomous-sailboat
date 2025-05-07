#include <Arduino.h>
#include <unity.h>
#include "xbeeImpl.h"  

xbeeImpl xbee;

// === TEST: sendValue (Serial Output) ===
void test_send() {
    String outputBuffer = "";

    class MockStream : public Stream {
        String &buffer;
    public:
        MockStream(String &buf) : buffer(buf) {}
        size_t write(uint8_t c) { buffer += (char)c; return 1; }
        int available() { return 0; }
        int read() { return -1; }
        int peek() { return -1; }
    };

    MockStream mockSerial(outputBuffer);

    xbee.Send(100, 30, 60, 90, mockSerial);

    TEST_ASSERT_TRUE(outputBuffer.indexOf("current:30") >= 0);
    TEST_ASSERT_TRUE(outputBuffer.indexOf("current:30") >= 0);
    TEST_ASSERT_TRUE(outputBuffer.indexOf("target:60") >= 0);
    TEST_ASSERT_TRUE(outputBuffer.indexOf("servo:90") >= 0);
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_send);
    UNITY_END();
}