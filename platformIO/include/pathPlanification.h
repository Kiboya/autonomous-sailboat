#ifndef PATH_PLANIFICATION_H
#define PATH_PLANIFICATION_H

#include <Arduino.h>
#include <math.h>
#include <vector>

#ifndef PI
#define PI 3.14159265358979323846
#endif

/**
 * @brief Advanced Layline-based Path Planner for sailboat navigation
 * 
 * This class implements a sophisticated upwind sailing strategy using layline tactics,
 * VMG optimization, and intelligent tacking decisions with confirmation logic.
 */
class LaylinePathPlanner {
private:
    // Constants for path planning behavior
    static constexpr double WAYPOINT_ARRIVAL_DISTANCE = 15.0;        // Distance threshold for waypoint arrival (meters)
    static constexpr double WAYPOINT_TIGHT_ARRIVAL_DISTANCE = 7.0;   // Close approach distance (meters)
    static constexpr double DECISION_COOLDOWN = 4.0;                 // Cooldown period after tack decisions (seconds)
    static constexpr int TACK_CONFIRMATION_THRESHOLD = 5;            // Required confirmations before tacking
    static constexpr int HEADING_HISTORY_SIZE = 5;                  // Size of heading history for smoothing
    static constexpr double TACK_HYSTERESIS_ANGLE_MARGIN = 8.0;     // Additional margin before layline crossing (degrees)
    static constexpr double HEADING_SMOOTHING_FACTOR = 0.3;         // Base smoothing factor for heading changes
    static constexpr double NO_GO_ZONE_BUFFER = 7.0;               // Buffer added to no-go zone (degrees)
    static constexpr double MINIMUM_INITIAL_DISTANCE = 15.0;        // Minimum distance before first tack (meters)
    static constexpr double MINIMUM_INITIAL_TIME = 7.0;            // Minimum time before first tack (seconds)

    // State variables for tacking logic
    bool current_tack_is_port;           // Current tack: true=port, false=starboard, null=direct sailing
    bool current_tack_is_set;            // Flag to indicate if current_tack_is_port is valid
    bool pending_tack_is_port;           // Pending tack being confirmed
    bool pending_tack_is_set;            // Flag to indicate if pending_tack_is_port is valid
    int tack_confirmation_count;         // Count of consistent tack confirmations
    
    // Timing and position tracking
    double last_decision_time;           // Time of last major decision
    double last_optimal_heading;         // Last smoothed optimal heading
    bool last_optimal_heading_set;       // Flag to indicate if last_optimal_heading is valid
    double last_raw_optimal_heading;     // Last raw heading before smoothing
    bool last_raw_optimal_heading_set;   // Flag to indicate if last_raw_optimal_heading is valid
    
    // Leg tracking for beginning protection
    double initial_lat, initial_lon;     // Starting position of current upwind leg
    double initial_time;                 // Starting time of current upwind leg
    bool initial_tack_chosen_for_leg;    // Flag indicating first tack has been chosen for this leg
    bool leg_initialized;                // Flag indicating leg tracking is initialized
    
    // Heading smoothing
    std::vector<double> heading_history; // History of headings for moving average
    
    /**
     * @brief Calculate VMG-optimal tack angle based on polar performance
     * @param wind_speed Current wind speed (m/s)
     * @return Optimal tack angle relative to true wind (degrees)
     */
    double find_vmg_optimal_tack_angle(double wind_speed);
    
    /**
     * @brief Check if a point is in the no-go zone with optional buffer
     * @param boat_lat Current boat latitude
     * @param boat_lon Current boat longitude
     * @param point_lat Target point latitude
     * @param point_lon Target point longitude
     * @param wind_direction Wind direction (degrees)
     * @param wind_speed Wind speed (m/s)
     * @param buffer Additional buffer to apply to no-go zone (degrees)
     * @return true if point is in buffered no-go zone
     */
    bool is_point_in_no_go_zone_buffered(double boat_lat, double boat_lon, 
                                         double point_lat, double point_lon,
                                         double wind_direction, double wind_speed, 
                                         double buffer = 0.0);
    
    /**
     * @brief Apply smoothing to heading changes using moving average and adaptive blending
     * @param new_raw_heading New raw heading to smooth
     * @return Smoothed heading
     */
    double apply_heading_smoothing(double new_raw_heading);
    
    /**
     * @brief Core decision logic for optimal heading before smoothing
     * @param boat_lat Current boat latitude
     * @param boat_lon Current boat longitude
     * @param wpt_lat Waypoint latitude
     * @param wpt_lon Waypoint longitude
     * @param compass Current compass heading (degrees)
     * @param wind_vane_relative Wind direction relative to boat (degrees)
     * @param wind_speed Wind speed (m/s)
     * @param current_time Current time (seconds)
     * @return Raw optimal heading before smoothing
     */
    double calculate_raw_direction(double boat_lat, double boat_lon, double wpt_lat, double wpt_lon,
                                  double compass, double wind_vane_relative, double wind_speed, 
                                  double current_time);
    
    /**
     * @brief Reset conditions that mark the start of a new upwind leg
     */
    void reset_leg_start_conditions();

public:
    /**
     * @brief Constructor - Initialize all state variables
     */
    LaylinePathPlanner();
    
    /**
     * @brief Calculate optimal sailing direction using layline tactics
     * @param boat_lat Current boat latitude
     * @param boat_lon Current boat longitude
     * @param waypoint_lat Target waypoint latitude
     * @param waypoint_lon Target waypoint longitude
     * @param compass Current compass heading (degrees)
     * @param wind_vane Wind direction relative to boat (degrees)
     * @param wind_speed Wind speed (m/s)
     * @param current_time Current time (seconds)
     * @return Optimal heading (degrees)
     */
    double calculate_direction(double boat_lat, double boat_lon, double waypoint_lat, double waypoint_lon,
                              double compass, double wind_vane, double wind_speed, double current_time);
    
    /**
     * @brief Reset planner state for new waypoint or simulation reset
     */
    void reset_planner_state();
    
    // Utility functions (shared with base implementation)
    static double calculate_azimuth(double lat1, double lon1, double lat2, double lon2);
    static double calculate_distance(double lat1, double lon1, double lat2, double lon2);
    static void define_no_go_zone(double wind_direction, double wind_speed, double* min_angle, double* max_angle);
    static bool is_in_no_go_zone(double azimuth, double min_angle, double max_angle);
    static double get_boat_speed_from_polars(double wind_angle, double wind_speed);
};

#endif // PATH_PLANIFICATION_H
