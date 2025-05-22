#include <Arduino.h>
#include <unity.h>
#include "servoControl.h"  

servoControl controller;

// === TEST: calculateShortestPath ===
void test_calculateShortestPath() {
    TEST_ASSERT_EQUAL(10, controller.calculateShortestPath(350, 0));
    TEST_ASSERT_EQUAL(-10, controller.calculateShortestPath(10, 0));
    TEST_ASSERT_EQUAL(-20, controller.calculateShortestPath(180, 200));
    TEST_ASSERT_EQUAL(5, controller.calculateShortestPath(5, 10));
    TEST_ASSERT_EQUAL(-5, controller.calculateShortestPath(355, 350));
}

// === TEST: getValue (Message Parsing) ===
void test_getValue() {
    String msg1 = "cap:180";
    String msg2 = "kp:2.5";
    String msg3 = "ki:1.2";
    String msg4 = "invalid:100";
    String msg5 = "voile:100";

    controller.getValue(msg1);
    TEST_ASSERT_EQUAL(180, controller.getTargetAngle());

    controller.getValue(msg2);
    TEST_ASSERT_EQUAL_FLOAT(2.5, controller.getKp());

    controller.getValue(msg3);
    TEST_ASSERT_EQUAL_FLOAT(1.2, controller.getKi());

    controller.getValue(msg4);  // Should not modify targetAngle, Kp, or Ki
    TEST_ASSERT_EQUAL(180, controller.getTargetAngle());
    TEST_ASSERT_EQUAL_FLOAT(2.5, controller.getKp());
    TEST_ASSERT_EQUAL_FLOAT(1.2, controller.getKi());

    controller.getValue(msg5); 
    TEST_ASSERT_EQUAL(100, controller.getVoileTensionPosition());
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_calculateShortestPath);
    RUN_TEST(test_getValue);
    UNITY_END();
}

void loop() {
    // Leave empty
}
