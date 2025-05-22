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
    TEST_ASSERT_EQUAL_INT(90, controller.calculateShortestPath(0, 90));
    TEST_ASSERT_EQUAL_INT(-90, controller.calculateShortestPath(180, 90));
    TEST_ASSERT_EQUAL_INT(-179, controller.calculateShortestPath(180, 1));
    TEST_ASSERT_EQUAL_INT(1, controller.calculateShortestPath(359, 0));
    TEST_ASSERT_EQUAL_INT(180, controller.calculateShortestPath(0, 180));
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_calculateShortestPath);
    UNITY_END();
}

void loop() {
    // Leave empty
}
