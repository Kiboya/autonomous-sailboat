#include "pathPlanification.h"
#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846
#define NoGoZone 45.00
#define threshold 10.00

// Function to calculate the azimuth towards the waypoint
double calculate_azimuth(double boat_lat, double boat_lon, double waypoint_lat, double waypoint_lon) {
    // Calculate the difference in longitude between the waypoint and the boat
    double dLon = (waypoint_lon - boat_lon) * (PI / 180.0);
    
    // Convert the boat's latitude and the waypoint's latitude from degrees to radians
    double lat1 = boat_lat * (PI / 180.0);
    double lat2 = waypoint_lat * (PI / 180.0);

    // Calculate the components of the azimuth
    double y = sin(dLon) * cos(lat2);
    double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon);
    
    // Calculate the azimuth in radians and convert it to degrees
    double azimuth = atan2(y, x) * (180.0 / PI);

    // Ensure the azimuth value is between 0 and 360 degrees
    return fmod(azimuth + 360.0, 360.0);
}

// Function to define the no-go zone due to wind direction
/**
 * @brief Defines the no-go zone based on the wind direction.
 *
 * This function calculates the no-go zone for a sailboat, which is the range of angles 
 * where the sailboat cannot sail effectively due to the wind direction. The no-go zone 
 * is typically 45 degrees to the left and right of the wind direction.
 *
 * @param wind_vane The wind direction in degrees (0 to 360).
 * @param min_angle Pointer to a double where the minimum angle of the no-go zone will be stored.
 * @param max_angle Pointer to a double where the maximum angle of the no-go zone will be stored.
 */
void define_no_go_zone(double wind_vane, double *min_angle, double *max_angle) {
    // Normalize the wind vane angle to be within 0 to 360 degrees
    double wind_abs = fmod(wind_vane + 360.0, 360.0);
    
    // Calculate the minimum angle of the no-go zone, 45 degrees to the left of the wind direction
    *min_angle = fmod(wind_abs - NoGoZone + 360.0, 360.0);
    
    // Calculate the maximum angle of the no-go zone, 45 degrees to the right of the wind direction
    *max_angle = fmod(wind_abs + NoGoZone, 360.0);
}

// Function to check if the azimuth is in the no-go zone
/**
 * @brief Checks if the azimuth is within the no-go zone.
 *
 * This function determines whether the given azimuth angle falls within the no-go zone 
 * defined by the minimum and maximum angles. The no-go zone is the range of angles where 
 * the sailboat cannot sail effectively due to the wind direction.
 *
 * @param azimuth The azimuth angle to check.
 * @param min_angle The minimum angle of the no-go zone.
 * @param max_angle The maximum angle of the no-go zone.
 * @return 1 if the azimuth is within the no-go zone, 0 otherwise.
 */
int is_in_no_go_zone(double azimuth, double min_angle, double max_angle) {
    // If the no-go zone does not cross the 0-degree line
    if (min_angle < max_angle)
        // Check if the azimuth is between the minimum and maximum angles
        return (azimuth >= min_angle && azimuth <= max_angle);
    else
        // If the no-go zone crosses the 0-degree line, check if the azimuth is outside the no-go zone
        return (azimuth >= min_angle || azimuth <= max_angle);
}

// Function to determine the optimal sailing direction
double calculate_direction(double boat_lat, double boat_lon, double waypoint_lat, double waypoint_lon,
    double horizontal_tilt, double vertical_tilt, double compass, double wind_vane) {
    // Calculate the azimuth towards the waypoint
    double azimuth = calculate_azimuth(boat_lat, boat_lon, waypoint_lat, waypoint_lon);

    // Adjust the wind direction relative to the North by combining compass and wind vane readings
    double wind_direction = fmod(compass + wind_vane, 360.0);

    // Define the no-go zone based on the adjusted wind direction
    double min_angle, max_angle;
    define_no_go_zone(wind_direction, &min_angle, &max_angle);

    // Check if the azimuth is outside the no-go zone
    if (!is_in_no_go_zone(azimuth, min_angle, max_angle)) {
        // If the azimuth is not in the no-go zone, return it as the optimal direction
        return azimuth;
    }

    // If the azimuth is in the no-go zone, find an alternative route
    // Calculate an escape route 10 degrees outside the maximum angle of the no-go zone
    double escape1 = fmod(max_angle + threshold, 360.0);
    // Calculate an escape route 10 degrees outside the minimum angle of the no-go zone
    double escape2 = fmod(min_angle - threshold + 360.0, 360.0);

    // Return the escape route that is closest to the original azimuth
    return (fabs(escape1 - azimuth) < fabs(escape2 - azimuth)) ? escape1 : escape2;
}


/*
int main() {
    double boat_lat = 48.8566, boat_lon = 2.3522;
    double waypoint_lat = 48.8570, waypoint_lon = 2.3530;
    double horizontal_tilt = 0.0, vertical_tilt = 0.0;
    double compass = 90.0, wind_vane = 180.0;

    double direction = calculate_direction(boat_lat, boat_lon, waypoint_lat, waypoint_lon, 
                                          horizontal_tilt, vertical_tilt, compass, wind_vane);
    printf("Optimal direction: %.2f°\n", direction);
    return 0;
}*/

