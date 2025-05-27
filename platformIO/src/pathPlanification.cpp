#include "pathPlanification.h"

/**
 * @brief Constructor - Initialize LaylinePathPlanner with default state
 */
LaylinePathPlanner::LaylinePathPlanner() {
    // Initialize tacking state
    current_tack_is_set = false;
    pending_tack_is_set = false;
    tack_confirmation_count = 0;
    
    // Initialize timing and heading state
    last_decision_time = 0.0;
    last_optimal_heading_set = false;
    last_raw_optimal_heading_set = false;
    
    // Initialize leg tracking
    leg_initialized = false;
    initial_tack_chosen_for_leg = false;
    
    // Initialize heading history vector
    heading_history.clear();
    heading_history.reserve(HEADING_HISTORY_SIZE);
}

/**
 * @brief Calculate VMG-optimal tack angle with wind compensation
 * 
 * Tests various tack angles to find the one that maximizes Velocity Made Good (VMG)
 * upwind, accounting for boat polars and wind conditions.
 */
double LaylinePathPlanner::find_vmg_optimal_tack_angle(double wind_speed) {
    double best_vmg = -INFINITY;
    double optimal_angle_to_true_wind = 50.0;  // Safe default
    
    // Test range of upwind sailing angles from 35 to 70 degrees
    for (int angle_deg = 35; angle_deg <= 70; angle_deg += 5) {
        double boat_speed = get_boat_speed_from_polars(angle_deg, wind_speed);
        // VMG = boat_speed * cos(angle_to_wind)
        double vmg = boat_speed * cos(angle_deg * PI / 180.0);
        
        if (vmg > best_vmg) {
            best_vmg = vmg;
            optimal_angle_to_true_wind = angle_deg;
        }
    }
    
    // Add safety buffer that increases with wind speed
    double base_buffer = 5.0;
    double wind_buffer = 0.0;
    
    if (wind_speed > 6.0) {
        // More buffer in stronger winds where drift is more pronounced
        wind_buffer = (wind_speed - 6.0) * 0.8;
    }
    
    // Ensure minimum safe angle and apply buffers
    return fmax(optimal_angle_to_true_wind, 40.0) + base_buffer + wind_buffer;
}

/**
 * @brief Check if point is in no-go zone with additional buffer
 * 
 * Enhanced no-go zone checking that allows for conservative navigation
 * by adding a buffer to the standard no-go zone.
 */
bool LaylinePathPlanner::is_point_in_no_go_zone_buffered(double boat_lat, double boat_lon,
                                                        double point_lat, double point_lon,
                                                        double wind_direction, double wind_speed,
                                                        double buffer) {
    double azimuth = calculate_azimuth(boat_lat, boat_lon, point_lat, point_lon);
    
    // Get base no-go zone angles
    double min_angle, max_angle;
    define_no_go_zone(wind_direction, wind_speed, &min_angle, &max_angle);
    
    // Calculate half-angle of no-go zone
    double current_no_go_angle;
    if (max_angle > min_angle) {
        current_no_go_angle = (max_angle - min_angle) / 2.0;
    } else {
        // Handle case where no-go zone crosses 0 degrees
        current_no_go_angle = (max_angle + (360.0 - min_angle)) / 2.0;
    }
    
    // Apply buffer to create more conservative no-go zone
    double effective_no_go_check_angle = current_no_go_angle + buffer;
    double min_angle_check = fmod(wind_direction - effective_no_go_check_angle + 360.0, 360.0);
    double max_angle_check = fmod(wind_direction + effective_no_go_check_angle, 360.0);
    
    return is_in_no_go_zone(azimuth, min_angle_check, max_angle_check);
}

/**
 * @brief Apply adaptive heading smoothing using moving average
 * 
 * Implements sophisticated smoothing that adapts to the magnitude of heading changes.
 * Small oscillations are heavily smoothed while large changes (like tacks) are less smoothed.
 */
double LaylinePathPlanner::apply_heading_smoothing(double new_raw_heading) {
    if (isnan(new_raw_heading)) {
        return last_optimal_heading_set ? last_optimal_heading : 0.0;
    }
    
    // Add new heading to history
    heading_history.push_back(new_raw_heading);
    if (heading_history.size() > HEADING_HISTORY_SIZE) {
        heading_history.erase(heading_history.begin());
    }
    
    // Calculate moving average using circular mean for angles
    double current_avg_heading;
    if (heading_history.empty()) {
        current_avg_heading = new_raw_heading;
    } else {
        double sin_sum = 0.0, cos_sum = 0.0;
        for (double hdg : heading_history) {
            sin_sum += sin(hdg * PI / 180.0);
            cos_sum += cos(hdg * PI / 180.0);
        }
        current_avg_heading = fmod(atan2(sin_sum, cos_sum) * 180.0 / PI + 360.0, 360.0);
    }
    
    if (!last_optimal_heading_set) {
        last_optimal_heading = current_avg_heading;
        last_optimal_heading_set = true;
    } else {
        // Calculate shortest angular difference
        double angle_diff = fmod(current_avg_heading - last_optimal_heading + 180.0, 360.0) - 180.0;
        
        // Adaptive smoothing factor based on change magnitude
        double smoothing_factor_to_use = HEADING_SMOOTHING_FACTOR;
        
        if (fabs(angle_diff) < 5.0) {
            // Very small changes - apply heavy smoothing to reduce oscillations
            smoothing_factor_to_use = 0.1;
        } else if (fabs(angle_diff) < 15.0) {
            // Small changes - apply medium smoothing
            smoothing_factor_to_use = 0.2;
        } else if (fabs(angle_diff) > 60.0) {
            // Large changes (likely tacks) - apply faster smoothing
            smoothing_factor_to_use = 0.5;
        }
        
        // Apply smoothing with adaptive factor
        last_optimal_heading = fmod(last_optimal_heading + angle_diff * smoothing_factor_to_use + 360.0, 360.0);
    }
    
    return last_optimal_heading;
}

/**
 * @brief Reset conditions for start of new upwind leg
 * 
 * Called when starting navigation to a new waypoint or when switching
 * from direct sailing back to tacking mode.
 */
void LaylinePathPlanner::reset_leg_start_conditions() {
    leg_initialized = false;
    initial_tack_chosen_for_leg = false;
    current_tack_is_set = false;
    pending_tack_is_set = false;
    tack_confirmation_count = 0;
    
    Serial.println("DEBUG: Leg start conditions reset for new upwind navigation");
}

/**
 * @brief Core decision logic implementing sophisticated layline tactics
 * 
 * This is the heart of the layline path planner, implementing:
 * - Direct sailing vs tacking decisions
 * - VMG-optimal tacking angles
 * - Layline crossing detection with confirmation
 * - Wind push compensation
 * - Beginning-of-route protection against premature tacking
 */
double LaylinePathPlanner::calculate_raw_direction(double boat_lat, double boat_lon, double wpt_lat, double wpt_lon,
                                                  double compass, double wind_vane_relative, double wind_speed,
                                                  double current_time) {
    // Calculate key navigation parameters
    double vmg_tack_angle = find_vmg_optimal_tack_angle(wind_speed);
    double azimuth_to_wpt = calculate_azimuth(boat_lat, boat_lon, wpt_lat, wpt_lon);
    double wind_direction_abs = fmod(compass + wind_vane_relative + 360.0, 360.0);
    double port_tack_target_hdg = fmod(wind_direction_abs - vmg_tack_angle + 360.0, 360.0);
    double starboard_tack_target_hdg = fmod(wind_direction_abs + vmg_tack_angle + 360.0, 360.0);
    double distance_to_wpt = calculate_distance(boat_lat, boat_lon, wpt_lat, wpt_lon);
    
    // Cooldown check - prevent rapid decision changes
    if (last_decision_time > 0 && (current_time - last_decision_time < DECISION_COOLDOWN) && 
        last_raw_optimal_heading_set) {
        Serial.println("DEBUG: In decision cooldown, maintaining course");
        return last_raw_optimal_heading;
    }
    
    // Check if direct sailing is feasible (conservative no-go zone check)
    double practical_no_go_angle = 45.0 + NO_GO_ZONE_BUFFER;  // Base no-go + buffer
    bool can_sail_direct = !is_point_in_no_go_zone_buffered(boat_lat, boat_lon, wpt_lat, wpt_lon,
                                                           wind_direction_abs, wind_speed, NO_GO_ZONE_BUFFER);
    
    // Decision logic: Direct vs Tacking
    if (current_tack_is_set) {
        // Already on a tack - only switch to direct if very close to waypoint AND direct is clear
        if (can_sail_direct && distance_to_wpt < WAYPOINT_ARRIVAL_DISTANCE) {
            Serial.println("DEBUG: Switching from tacking to direct sailing near waypoint");
            reset_leg_start_conditions();
            last_decision_time = current_time;
            return azimuth_to_wpt;
        }
        // Continue with tacking logic below
    } else {
        // Not currently tacking
        if (can_sail_direct) {
            Serial.println("DEBUG: Direct sailing to waypoint");
            reset_leg_start_conditions();
            last_decision_time = current_time;
            return azimuth_to_wpt;
        } else {
            // Must initiate tacking - initialize leg tracking
            if (!leg_initialized) {
                initial_lat = boat_lat;
                initial_lon = boat_lon;
                initial_time = current_time;
                leg_initialized = true;
                Serial.println("DEBUG: Initializing new upwind leg");
            }
        }
    }
    
    // === TACKING LOGIC ===
    
    // Initial tack selection for this leg
    if (!current_tack_is_set) {
        if (!initial_tack_chosen_for_leg) {
            // Choose tack requiring minimal turning from current heading
            double port_hdg_diff = fabs(fmod(port_tack_target_hdg - compass + 180.0, 360.0) - 180.0);
            double stbd_hdg_diff = fabs(fmod(starboard_tack_target_hdg - compass + 180.0, 360.0) - 180.0);
            current_tack_is_port = (port_hdg_diff < stbd_hdg_diff);
            current_tack_is_set = true;
            initial_tack_chosen_for_leg = true;
            Serial.printf("DEBUG: Initial tack selected: %s\n", current_tack_is_port ? "PORT" : "STARBOARD");
        } else {
            // Fallback: choose based on waypoint bearing
            double angle_diff_port = fabs(fmod(port_tack_target_hdg - azimuth_to_wpt + 180.0, 360.0) - 180.0);
            double angle_diff_starboard = fabs(fmod(starboard_tack_target_hdg - azimuth_to_wpt + 180.0, 360.0) - 180.0);
            current_tack_is_port = (angle_diff_port < angle_diff_starboard);
            current_tack_is_set = true;
        }
        
        pending_tack_is_set = false;
        tack_confirmation_count = 0;
        last_decision_time = current_time;
        return current_tack_is_port ? port_tack_target_hdg : starboard_tack_target_hdg;
    }
    
    // Beginning of leg protection - prevent premature tacking
    if (leg_initialized && initial_tack_chosen_for_leg) {
        double distance_traveled = calculate_distance(boat_lat, boat_lon, initial_lat, initial_lon);
        double time_elapsed = current_time - initial_time;
        
        if (distance_traveled < MINIMUM_INITIAL_DISTANCE || time_elapsed < MINIMUM_INITIAL_TIME) {
            Serial.printf("DEBUG: Beginning protection active - traveled: %.1fm, elapsed: %.1fs\n", 
                         distance_traveled, time_elapsed);
            return current_tack_is_port ? port_tack_target_hdg : starboard_tack_target_hdg;
        }
    }
    
    // Layline crossing detection with enhanced margins
    double bearing_to_wpt = calculate_azimuth(boat_lat, boat_lon, wpt_lat, wpt_lon);
    double relative_wpt_bearing_to_wind = fmod(bearing_to_wpt - wind_direction_abs + 180.0, 360.0) - 180.0;
    
    // Dynamic layline margin calculation
    double wind_push_factor = (wind_speed > 5.0) ? fmin((wind_speed - 5.0) * 2.5, 20.0) : 0.0;
    double distance_factor = fmin(15.0, fmax(7.0, distance_to_wpt / 10.0));
    double effective_layline_check_angle = vmg_tack_angle + TACK_HYSTERESIS_ANGLE_MARGIN + 
                                         fmax(wind_push_factor, distance_factor);
    
    // Require more confirmations when far from waypoint
    int required_confirmation = TACK_CONFIRMATION_THRESHOLD;
    if (distance_to_wpt > 50.0) {
        required_confirmation = (int)(TACK_CONFIRMATION_THRESHOLD * 1.5);
    }
    
    // Check for layline crossing
    bool propose_tack_now = false;
    bool newly_proposed_tack_is_port = current_tack_is_port;
    
    if (current_tack_is_port) {
        // On port tack, check for crossing to starboard layline
        if (relative_wpt_bearing_to_wind > effective_layline_check_angle) {
            propose_tack_now = true;
            newly_proposed_tack_is_port = false;
        }
    } else {
        // On starboard tack, check for crossing to port layline
        if (relative_wpt_bearing_to_wind < -effective_layline_check_angle) {
            propose_tack_now = true;
            newly_proposed_tack_is_port = true;
        }
    }
    
    // Tack confirmation management
    if (propose_tack_now) {
        if (!pending_tack_is_set || pending_tack_is_port != newly_proposed_tack_is_port) {
            // New or different tack proposal
            pending_tack_is_port = newly_proposed_tack_is_port;
            pending_tack_is_set = true;
            tack_confirmation_count = 1;
            Serial.printf("DEBUG: Tack to %s proposed (conf %d/%d)\n", 
                         newly_proposed_tack_is_port ? "PORT" : "STARBOARD", 
                         tack_confirmation_count, required_confirmation);
        } else {
            // Same tack proposal continues
            tack_confirmation_count++;
            Serial.printf("DEBUG: Tack proposal continues (conf %d/%d)\n", 
                         tack_confirmation_count, required_confirmation);
        }
        
        if (tack_confirmation_count >= required_confirmation) {
            // CONFIRMED TACK
            Serial.printf("DEBUG: *** TACK CONFIRMED to %s ***\n", 
                         pending_tack_is_port ? "PORT" : "STARBOARD");
            current_tack_is_port = pending_tack_is_port;
            pending_tack_is_set = false;
            tack_confirmation_count = 0;
            last_decision_time = current_time;
        }
    } else {
        // No tack conditions met - reset pending if it existed
        if (pending_tack_is_set) {
            Serial.println("DEBUG: Tack conditions no longer met, resetting confirmation");
            pending_tack_is_set = false;
            tack_confirmation_count = 0;
        }
    }
    
    // Return heading based on current tack
    return current_tack_is_port ? port_tack_target_hdg : starboard_tack_target_hdg;
}

/**
 * @brief Main entry point for direction calculation with smoothing
 */
double LaylinePathPlanner::calculate_direction(double boat_lat, double boat_lon, double waypoint_lat, double waypoint_lon,
                                              double compass, double wind_vane, double wind_speed, double current_time) {
    // Get raw optimal heading from decision logic
    double raw_heading_decision = calculate_raw_direction(boat_lat, boat_lon, waypoint_lat, waypoint_lon,
                                                         compass, wind_vane, wind_speed, current_time);
    
    // Store raw heading for reference
    last_raw_optimal_heading = raw_heading_decision;
    last_raw_optimal_heading_set = true;
    
    // Apply smoothing and return final heading
    return apply_heading_smoothing(raw_heading_decision);
}

/**
 * @brief Reset planner state for new waypoint or simulation reset
 */
void LaylinePathPlanner::reset_planner_state() {
    reset_leg_start_conditions();
    last_optimal_heading_set = false;
    last_raw_optimal_heading_set = false;
    last_decision_time = 0.0;
    heading_history.clear();
    Serial.println("DEBUG: LaylinePathPlanner state completely reset");
}

// Static utility functions (shared with original implementation)

double LaylinePathPlanner::calculate_azimuth(double lat1, double lon1, double lat2, double lon2) {
    double dLon = (lon2 - lon1) * (PI / 180.0);
    double lat1_rad = lat1 * (PI / 180.0);
    double lat2_rad = lat2 * (PI / 180.0);
    
    double y = sin(dLon) * cos(lat2_rad);
    double x = cos(lat1_rad) * sin(lat2_rad) - sin(lat1_rad) * cos(lat2_rad) * cos(dLon);
    
    double azimuth = atan2(y, x) * (180.0 / PI);
    return fmod(azimuth + 360.0, 360.0);
}

double LaylinePathPlanner::calculate_distance(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371000;  // Earth radius in meters
    double lat1_rad = lat1 * PI / 180.0;
    double lat2_rad = lat2 * PI / 180.0;
    double dLat = (lat2 - lat1) * PI / 180.0;
    double dLon = (lon2 - lon1) * PI / 180.0;
    
    double a = sin(dLat/2) * sin(dLat/2) + 
               cos(lat1_rad) * cos(lat2_rad) * sin(dLon/2) * sin(dLon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    
    return R * c;
}

void LaylinePathPlanner::define_no_go_zone(double wind_direction, double wind_speed, double* min_angle, double* max_angle) {
    const double NO_GO_ZONE_ANGLE = 45.0;
    double wind_abs = fmod(wind_direction + 360.0, 360.0);
    
    // Adjust no-go zone based on wind speed
    double adjusted_no_go = NO_GO_ZONE_ANGLE;
    if (wind_speed > 15) {
        adjusted_no_go = NO_GO_ZONE_ANGLE * 1.2;  // Wider no-go in strong winds
    } else if (wind_speed < 5) {
        adjusted_no_go = NO_GO_ZONE_ANGLE * 0.8;  // Narrower no-go in light winds
    }
    
    *min_angle = fmod(wind_abs - adjusted_no_go + 360.0, 360.0);
    *max_angle = fmod(wind_abs + adjusted_no_go, 360.0);
}

bool LaylinePathPlanner::is_in_no_go_zone(double azimuth, double min_angle, double max_angle) {
    if (min_angle < max_angle) {
        return (azimuth >= min_angle && azimuth <= max_angle);
    } else {
        return (azimuth >= min_angle || azimuth <= max_angle);
    }
}

double LaylinePathPlanner::get_boat_speed_from_polars(double wind_angle, double wind_speed) {
    double abs_wind_angle = fabs(wind_angle);
    while (abs_wind_angle > 180) {
        abs_wind_angle = 360 - abs_wind_angle;
    }
    
    if (abs_wind_angle < 35) {
        return 0.0;  // Can't sail this close to the wind
    } else if (abs_wind_angle < 50) {
        return 0.5 * wind_speed * 0.4;  // Close-hauled, reduced from original
    } else if (abs_wind_angle < 90) {
        return 0.8 * wind_speed * 0.5;  // Reaching
    } else if (abs_wind_angle < 150) {
        return 1.0 * wind_speed * 0.6;  // Broad reach, fastest point of sail
    } else {
        return 0.7 * wind_speed * 0.5;  // Running
    }
}
