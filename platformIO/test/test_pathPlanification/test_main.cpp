#include <Arduino.h>  // Required for PlatformIO/Unity compatibility
#include <unity.h>
#include "pathPlanification.h"

void setUp(void) {
    // Runs before each test
}


void tearDown(void) {
    // Runs after each test
}

// ------------------------
// Test: calculate_azimuth
// ------------------------
void test_calculate_azimuth_basic(void) {
    double azimuth = calculate_azimuth(48.8566, 2.3522, 48.8570, 2.3530);
    TEST_ASSERT_FLOAT_WITHIN(0.5, 52.76, azimuth); // Adjust tolerance as needed
}

// ------------------------
// Test: define_no_go_zone
// ------------------------
void test_define_no_go_zone_basic(void) {
    double min_angle, max_angle;
    define_no_go_zone(90.0, &min_angle, &max_angle);
    TEST_ASSERT_FLOAT_WITHIN(0.1, 45.0, min_angle);
    TEST_ASSERT_FLOAT_WITHIN(0.1, 135.0, max_angle);
}

// ------------------------
// Test: is_in_no_go_zone
// ------------------------
void test_is_in_no_go_zone_inside(void) {
    TEST_ASSERT_TRUE(is_in_no_go_zone(90.0, 45.0, 135.0));
}

void test_is_in_no_go_zone_outside(void) {
    TEST_ASSERT_FALSE(is_in_no_go_zone(200.0, 45.0, 135.0));
}

// ------------------------
// Test: calculate_direction
// ------------------------
void test_calculate_direction_outside_no_go_zone(void) {
    double direction = calculate_direction(48.8566, 2.3522, 48.8570, 2.3530,
                                           0.0, 0.0, 90.0, 0.0);
    TEST_ASSERT_FLOAT_WITHIN(1.0, 52.76, direction);
}

void test_calculate_direction_inside_no_go_zone(void) {
    // Should return an escape route
    double direction = calculate_direction(48.8566, 2.3522, 48.8570, 2.3530,
                                           0.0, 0.0, 0.0, 0.0); // Wind from 0°, azimuth ≈ 52°, inside No-Go
    TEST_ASSERT_TRUE(direction != 52.76); // Should NOT be original azimuth
    TEST_ASSERT_TRUE(direction > 0.0 && direction < 360.0);
}

// ------------------------
// Unity Main Function
// ------------------------
void setup() {
    UNITY_BEGIN();

    RUN_TEST(test_calculate_azimuth_basic);
    RUN_TEST(test_define_no_go_zone_basic);
    RUN_TEST(test_is_in_no_go_zone_inside);
    RUN_TEST(test_is_in_no_go_zone_outside);
    RUN_TEST(test_calculate_direction_outside_no_go_zone);
    RUN_TEST(test_calculate_direction_inside_no_go_zone);

    UNITY_END();
}

void loop() {
    // Required by Arduino framework
}


