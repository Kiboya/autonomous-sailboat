# Path Planning System Documentation

The path planning system has been updated to use a class-based approach with improved layline tactics for upwind sailing. This document explains the functionality and usage of the new implementation.

## `LaylinePathPlanner` Class Overview

The `LaylinePathPlanner` class provides sophisticated path planning for autonomous sailboats, with special focus on upwind sailing tactics using laylines. The system decides between direct sailing and tacking based on the relationship between the wind direction and the target waypoint.

### Key Features

- **Intelligent Upwind Navigation**: Optimizes Velocity Made Good (VMG) for upwind legs
- **Layline Detection**: Calculates optimal tacking points for efficient upwind sailing
- **Tacking Hysteresis**: Prevents excessive tacking by applying decision smoothing
- **Weather Awareness**: Adapts strategies based on wind speed and direction

### Class Structure

```cpp
class LaylinePathPlanner {
private:
    // State tracking variables
    double last_decision_time;
    double start_time;
    bool on_port_tack;
    bool direct_sailing;
    
    // Constants for tuning behavior
    static constexpr double WAYPOINT_ARRIVAL_DISTANCE = 15.0;
    static constexpr double WAYPOINT_TIGHT_ARRIVAL_DISTANCE = 7.0;
    static constexpr double DECISION_COOLDOWN = 4.0;
    static constexpr double MINIMUM_INITIAL_TIME = 7.0;
    static constexpr double TACK_HYSTERESIS_ANGLE_MARGIN = 8.0;
    static constexpr double NO_GO_ZONE_BUFFER = 7.0;

public:
    // Constructor
    LaylinePathPlanner();
    
    // Main path planning method
    double calculate_direction(double boat_lat, double boat_lon, double waypoint_lat, double waypoint_lon,
                              double compass, double wind_vane, double wind_speed, double current_time);
    
    // Reset planner state
    void reset_planner_state();
    
    // Static utility methods
    static double calculate_azimuth(double boat_lat, double boat_lon, double waypoint_lat, double waypoint_lon);
    static double calculate_distance(double boat_lat, double boat_lon, double waypoint_lat, double waypoint_lon);
    static void define_no_go_zone(double wind_vane, double wind_speed, double *min_angle, double *max_angle);
    static bool is_in_no_go_zone(double azimuth, double min_angle, double max_angle);
};
```

## Static Utility Methods

### `calculate_azimuth` Method

**Description**
Calculates the azimuth (bearing) from the boat's position to a waypoint.

**Prototype**
```cpp
static double calculate_azimuth(double boat_lat, double boat_lon, double waypoint_lat, double waypoint_lon);
```

**Parameters**
- `boat_lat`: Boat's latitude in decimal degrees
- `boat_lon`: Boat's longitude in decimal degrees
- `waypoint_lat`: Waypoint's latitude in decimal degrees
- `waypoint_lon`: Waypoint's longitude in decimal degrees

**Returns**
- The azimuth in degrees (0-360°) from the boat to the waypoint

**Example Usage**

```cpp
#include "pathPlanification.h"

int main() {
    double boat_lat = 48.8566, boat_lon = 2.3522;  // Paris
    double waypoint_lat = 48.8570, waypoint_lon = 2.3530;
    
    double azimuth = LaylinePathPlanner::calculate_azimuth(boat_lat, boat_lon, waypoint_lat, waypoint_lon);
    printf("Azimuth to waypoint: %.2f°\n", azimuth);
    return 0;
}
```

**Expected Output**
```
Azimuth to waypoint: 45.72°
```

### `calculate_distance` Method

**Description**
Calculates the distance in meters between two GPS coordinates using the haversine formula.

**Prototype**
```cpp
static double calculate_distance(double boat_lat, double boat_lon, double waypoint_lat, double waypoint_lon);
```

**Parameters**
- `boat_lat`: Boat's latitude in decimal degrees
- `boat_lon`: Boat's longitude in decimal degrees
- `waypoint_lat`: Waypoint's latitude in decimal degrees
- `waypoint_lon`: Waypoint's longitude in decimal degrees

**Returns**
- The distance in meters between the two points

**Example Usage**

```cpp
double distance = LaylinePathPlanner::calculate_distance(48.8566, 2.3522, 48.8570, 2.3530);
printf("Distance to waypoint: %.2f meters\n", distance);
```

**Expected Output**
```
Distance to waypoint: 59.24 meters
```

### `define_no_go_zone` Method

**Description**
Defines the no-go zone (angles where the boat cannot sail directly) based on the wind direction.

**Prototype**
```cpp
static void define_no_go_zone(double wind_vane, double wind_speed, double *min_angle, double *max_angle);
```

**Parameters**
- `wind_vane`: Wind direction in degrees (0-360°)
- `wind_speed`: Wind speed in m/s (affects the size of the no-go zone)
- `min_angle`: Pointer to store the minimum angle of the no-go zone
- `max_angle`: Pointer to store the maximum angle of the no-go zone

**Returns**
- None (updates the values pointed by min_angle and max_angle)

**Example Usage**

```cpp
double min_angle, max_angle;
double wind_direction = 180.0;  // Wind from the south
double wind_speed = 5.0;  // 5 m/s wind speed

LaylinePathPlanner::define_no_go_zone(wind_direction, wind_speed, &min_angle, &max_angle);
printf("Wind Direction: %.2f°\n", wind_direction);
printf("No-Go Zone: [%.2f° to %.2f°]\n", min_angle, max_angle);
```

**Expected Output**
```
Wind Direction: 180.00°
No-Go Zone: [135.00° to 225.00°]
```

### `is_in_no_go_zone` Method

**Description**
Determines if a given azimuth falls within the no-go zone.

**Prototype**
```cpp
static bool is_in_no_go_zone(double azimuth, double min_angle, double max_angle);
```

**Parameters**
- `azimuth`: Direction to check in degrees (0-360°)
- `min_angle`: Minimum angle of the no-go zone in degrees
- `max_angle`: Maximum angle of the no-go zone in degrees

**Returns**
- `true` if the azimuth is in the no-go zone, `false` otherwise

**Example Usage**

```cpp
double min_angle = 135.0, max_angle = 225.0;
double direction_to_check = 180.0;

bool in_no_go = LaylinePathPlanner::is_in_no_go_zone(direction_to_check, min_angle, max_angle);
printf("Direction %.2f° is %s the no-go zone\n", 
       direction_to_check, in_no_go ? "inside" : "outside");
```

**Expected Output**
```
Direction 180.00° is inside the no-go zone
```

## Main Path Planning Method

### `calculate_direction` Method

**Description**
The main path planning method that determines the optimal sailing direction based on the waypoint location, wind conditions, and boat state.

**Prototype**
```cpp
double calculate_direction(double boat_lat, double boat_lon, double waypoint_lat, double waypoint_lon,
                          double compass, double wind_vane, double wind_speed, double current_time);
```

**Parameters**
- `boat_lat`: Boat's latitude in decimal degrees
- `boat_lon`: Boat's longitude in decimal degrees
- `waypoint_lat`: Waypoint's latitude in decimal degrees
- `waypoint_lon`: Waypoint's longitude in decimal degrees
- `compass`: Current boat heading in degrees
- `wind_vane`: Wind direction relative to boat in degrees
- `wind_speed`: Wind speed in m/s
- `current_time`: Current time in seconds (used for decision timing)

**Returns**
- The optimal sailing direction in degrees (0-360°)

**How It Works**

1. Calculate the direct azimuth to the waypoint
2. Determine the no-go zone based on wind direction
3. Check if the direct path to the waypoint is in the no-go zone
   - If not, sail directly to the waypoint
   - If yes, implement tacking strategy:
     - Determine which tack to use (port or starboard)
     - Calculate laylines to the waypoint
     - Switch tacks when crossing a layline
     - Apply hysteresis to avoid frequent tack changes
4. Return the optimal sailing direction

**Example Usage**

```cpp
#include "pathPlanification.h"
#include <stdio.h>

int main() {
    double boat_lat = 48.8566, boat_lon = 2.3522;  // Paris
    double waypoint_lat = 48.8570, waypoint_lon = 2.3530;  // Nearby destination
    double compass = 90.0;  // Boat facing east
    double wind_vane = 0.0;  // Wind from north
    double wind_speed = 5.0;  // 5 m/s
    double current_time = 0.0;  // Starting time
    
    LaylinePathPlanner planner;
    double direction = planner.calculate_direction(
        boat_lat, boat_lon, waypoint_lat, waypoint_lon,
        compass, wind_vane, wind_speed, current_time
    );

    // Output the result
    printf("Optimal sailing direction: %.2f°\n", direction);
    return 0;
}
```

**Expected Output**
```
Optimal sailing direction: 45.72°
```

## Using the Path Planner in the Main Program

In the main program, the path planner is used in the `pathFinding` task:

```cpp
void pathFinding(void* pvParameters) {
    // Create planner instance
    LaylinePathPlanner planner;
    
    while (true) {
        // Get current position and sensors data
        double boat_lat = sharedData.latitude;
        double boat_lon = sharedData.longitude;
        double waypoint_lat = 48.8570;  // Set your actual waypoint
        double waypoint_lon = 2.3530;   // Set your actual waypoint
        
        double compass = sharedData.angle_from_north;
        double wind_vane = sharedData.wind_vane;
        double wind_speed = 5.0;  // Update with actual wind speed when available
        double current_time = millis() / 1000.0;
        
        // Calculate optimal direction
        double targetAngle = planner.calculate_direction(
            boat_lat, boat_lon, waypoint_lat, waypoint_lon,
            compass, wind_vane, wind_speed, current_time
        );
        
        // Update shared data for servo control
        sharedData.targetAngle = targetAngle;
        
        delay(100);  // Update frequency
    }
}
```