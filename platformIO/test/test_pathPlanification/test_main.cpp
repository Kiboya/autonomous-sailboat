#include <Arduino.h>  // Required for PlatformIO/Unity compatibility
#include <unity.h>
#include "pathPlanification.h"

// Test fixture
LaylinePathPlanner planner;

void setUp(void) {
    // Reset planner state before each test
    planner.reset_planner_state();
}

void tearDown(void) {
    // Clean up after each test
}

// ------------------------
// Test: Static Utility Functions
// ------------------------
void test_calculate_azimuth(void) {
    double azimuth = LaylinePathPlanner::calculate_azimuth(48.8566, 2.3522, 48.8570, 2.3530);
    TEST_ASSERT_FLOAT_WITHIN(0.5, 52.76, azimuth); // Adjust tolerance as needed
}

void test_calculate_distance(void) {
    double distance = LaylinePathPlanner::calculate_distance(48.8566, 2.3522, 48.8570, 2.3530);
    // ~50-60m distance depending on exact calculation
    TEST_ASSERT_TRUE(distance > 45.0 && distance < 65.0);
}

void test_define_no_go_zone(void) {
    double min_angle, max_angle;
    LaylinePathPlanner::define_no_go_zone(90.0, 5.0, &min_angle, &max_angle);
    TEST_ASSERT_FLOAT_WITHIN(0.1, 45.0, min_angle);
    TEST_ASSERT_FLOAT_WITHIN(0.1, 135.0, max_angle);
}

void test_is_in_no_go_zone(void) {
    // Test point inside no-go zone
    TEST_ASSERT_TRUE(LaylinePathPlanner::is_in_no_go_zone(90.0, 45.0, 135.0));
    
    // Test point outside no-go zone
    TEST_ASSERT_FALSE(LaylinePathPlanner::is_in_no_go_zone(200.0, 45.0, 135.0));
    
    // Test with zone wrapping around north
    TEST_ASSERT_TRUE(LaylinePathPlanner::is_in_no_go_zone(350.0, 340.0, 10.0));
    TEST_ASSERT_FALSE(LaylinePathPlanner::is_in_no_go_zone(20.0, 340.0, 10.0));
}

// ------------------------
// Test: Direct Path vs. Tacking Decision
// ------------------------
void test_direct_sailing_when_possible(void) {
    // Waypoint is NOT in no-go zone relative to wind
    double direction = planner.calculate_direction(
        48.8566, 2.3522,     // boat position
        48.8600, 2.3700,     // waypoint (~1km east-northeast)
        90.0,                // compass heading (east)
        270.0,               // wind from west (relative to boat)
        5.0,                 // wind speed 5 m/s
        0.0                  // current time
    );
    
    // Direction should be close to direct route since wind is favorable
    double direct_azimuth = LaylinePathPlanner::calculate_azimuth(48.8566, 2.3522, 48.8600, 2.3700);
    TEST_ASSERT_FLOAT_WITHIN(10.0, direct_azimuth, direction);
}

void test_tacking_when_necessary(void) {
    // Waypoint IS in no-go zone relative to wind (wind from direction of waypoint)
    double direction = planner.calculate_direction(
        48.8566, 2.3522,     // boat position
        48.8600, 2.3530,     // waypoint (north)
        0.0,                 // compass heading (north)
        0.0,                 // wind from north (relative to boat)
        5.0,                 // wind speed 5 m/s
        0.0                  // current time
    );
    
    // Direct azimuth would be 0 degrees (north), but we can't sail there
    // Direction should be on one of the tacks, not directly toward waypoint
    double direct_azimuth = LaylinePathPlanner::calculate_azimuth(48.8566, 2.3522, 48.8600, 2.3530);
    TEST_ASSERT_TRUE(fabs(direction - direct_azimuth) > 20.0);
    
    // Direction should be one of the optimal tack angles
    // Either port tack (315° ±20°) or starboard tack (45° ±20°)
    bool is_valid_tack = (fabs(fmod(direction - 315.0 + 360.0, 360.0)) < 20.0) || 
                         (fabs(fmod(direction - 45.0 + 360.0, 360.0)) < 20.0);
    TEST_ASSERT_TRUE(is_valid_tack);
}

// ------------------------
// Unity Main Function
// ------------------------
void setup() {
    delay(2000);  // Give serial port time to connect
    UNITY_BEGIN();

    // Test static utility functions
    RUN_TEST(test_calculate_azimuth);
    RUN_TEST(test_calculate_distance);
    RUN_TEST(test_define_no_go_zone);
    RUN_TEST(test_is_in_no_go_zone);
    
    // Test path planning behavior
    RUN_TEST(test_direct_sailing_when_possible);
    RUN_TEST(test_tacking_when_necessary);

    UNITY_END();
}

void loop() {
    // Required by Arduino framework
}