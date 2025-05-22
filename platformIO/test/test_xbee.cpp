#include <Arduino.h>
#include <unity.h>
#include "xbeeImpl.h"
#include "shared_data.h"

// Mock Serial1
class MockSerial {
public:
    String output = "";

    void print(const char* msg) { output += String(msg); }
    void print(double val, int precision) { output += String(val, precision); }
    void print(int val) { output += String(val); }
    void println(const char* msg) { output += String(msg) + "\n"; }
    void println(double val, int precision) { output += String(val, precision) + "\n"; }
    void println(int val) { output += String(val) + "\n"; }

    void clear() { output = ""; }
};

MockSerial mockSerial;
#define Serial1 mockSerial  // Override Serial1 used in xbeeImpl

xbeeImpl xbee;  // This uses the overridden Serial1

// === TEST: sendValue (Serial Output) ===
void test_send_changed_values() {
    SharedData data = {
        .latitude = 48.8566,
        .longitude = 2.3522,
        .compass = 123.45,
        .wind_vane = 234.56,
        .horizontal_tilt = 1.23,
        .vertical_tilt = 4.56,
        .targetAngle = 90,
        .targetTension = 10,
        .angleFromNorth = 45
    };

    mockSerial.clear();
    xbee.send(data);

    TEST_ASSERT_TRUE_MESSAGE(mockSerial.output.indexOf("latitude: 48.856600") >= 0, "Latitude not sent correctly");
    TEST_ASSERT_TRUE_MESSAGE(mockSerial.output.indexOf("longitude: 2.352200") >= 0, "Longitude not sent correctly");
    TEST_ASSERT_TRUE_MESSAGE(mockSerial.output.indexOf("compass: 123.45") >= 0, "Compass not sent correctly");
    TEST_ASSERT_TRUE_MESSAGE(mockSerial.output.indexOf("wind vane: 234.56") >= 0, "Wind vane not sent correctly");
    TEST_ASSERT_TRUE_MESSAGE(mockSerial.output.indexOf("horizontal tilt: 1.23") >= 0, "Horizontal tilt not sent correctly");
    TEST_ASSERT_TRUE_MESSAGE(mockSerial.output.indexOf("vertical tilt: 4.56") >= 0, "Vertical tilt not sent correctly");
    TEST_ASSERT_TRUE_MESSAGE(mockSerial.output.indexOf("target angle: 90") >= 0, "Target angle not sent correctly");
    TEST_ASSERT_TRUE_MESSAGE(mockSerial.output.indexOf("target tension: 10") >= 0, "Target tension not sent correctly");
    TEST_ASSERT_TRUE_MESSAGE(mockSerial.output.indexOf("angle from north: 45") >= 0, "Angle from north not sent correctly");
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_send_changed_values);
    UNITY_END();
}