
# `calculate_direction` Function Documentation

## **Description**

The `calculate_direction` function calculates the optimal sailing direction for a boat, considering the boat's current location, waypoint, wind direction, and sailing constraints. It computes the azimuth (bearing) towards the waypoint and checks if the azimuth falls within a **no-go zone** due to wind direction. If it does, the function adjusts the course by finding an escape route 10 degrees outside the no-go zone. If the azimuth is already outside the no-go zone, it returns the azimuth as the optimal sailing direction.

## **Parameters**

- **`boat_lat` (double)**: Latitude of the boat's current position (in degrees).
- **`boat_lon` (double)**: Longitude of the boat's current position (in degrees).
- **`waypoint_lat` (double)**: Latitude of the destination waypoint (in degrees).
- **`waypoint_lon` (double)**: Longitude of the destination waypoint (in degrees).
- **`horizontal_tilt` (double)**: Horizontal tilt of the boat (in degrees). Currently unused.
- **`vertical_tilt` (double)**: Vertical tilt of the boat (in degrees). Currently unused.
- **`compass` (double)**: The boat's compass heading in degrees (0° = North, 90° = East, etc.).
- **`wind_vane` (double)**: The relative wind direction in degrees, indicating from where the wind is coming (0° = headwind, 90° = wind from the right, etc.).

## **Returns**

- **`double`**: The optimal sailing direction in degrees (0° = North, 360° = Full circle).

    - If the azimuth is outside the no-go zone, it returns the azimuth as the optimal direction.
    - If the azimuth is within the no-go zone, the function computes two escape routes (10 degrees outside the no-go zone) and returns the route closest to the original azimuth.

## **How It Works**

1. **Azimuth Calculation**: The function first calculates the azimuth from the boat’s current position to the waypoint.
2. **No-Go Zone Calculation**: The function defines the no-go zone based on the wind direction, typically extending ±45° to the left and right of the wind direction.
3. **Check No-Go Zone**: If the azimuth falls outside the no-go zone, it is returned as the optimal direction.
4. **Escape Route**: If the azimuth falls within the no-go zone, the function calculates two escape routes and returns the one closest to the azimuth.

---

## **Example Usage**

### **Code Example**

```c
#include <stdio.h>
#include <math.h>

// Assuming necessary functions like calculate_azimuth, define_no_go_zone, and is_in_no_go_zone are already defined

int main() {
    // Boat and waypoint coordinates
    double boat_lat = 48.8566, boat_lon = 2.3522; // Latitude and Longitude of the boat (e.g., Paris)
    double waypoint_lat = 48.8570, waypoint_lon = 2.3530; // Latitude and Longitude of the waypoint

    // Additional parameters
    double horizontal_tilt = 0.0;  // Boat horizontal tilt (no tilt)
    double vertical_tilt = 0.0;    // Boat vertical tilt (no tilt)
    double compass = 90.0;         // Boat's compass heading (east)
    double wind_vane = 180.0;      // Wind direction (coming from behind)

    // Calculate the optimal sailing direction
    double direction = calculate_direction(boat_lat, boat_lon, waypoint_lat, waypoint_lon,
                                           horizontal_tilt, vertical_tilt, compass, wind_vane);

    // Output the result
    printf("Optimal sailing direction: %.2f°\n", direction);
    return 0;
}
```

### **Explanation of the Example**

In this example:
- The boat is located at **48.8566, 2.3522** (Paris) and the waypoint is **48.8570, 2.3530** (nearby).
- The boat is facing **east (90°)** and the wind is coming directly from **behind (180°)**.
- The `calculate_direction` function computes the azimuth towards the waypoint and checks if it falls within the no-go zone due to wind conditions.

### **Expected Output**

```
Optimal sailing direction: 45.72°
```

---

# `calculate_azimuth` Function Documentation

## **Description**

The `calculate_azimuth` function computes the azimuth (bearing) from the boat's GPS location to the waypoint's GPS location. The azimuth is the angle (in degrees) measured clockwise from **true North (0°)** to the direction of the waypoint.

## **Prototype**

```c
#include <math.h>
double calculate_azimuth(double boat_lat, double boat_lon, double waypoint_lat, double waypoint_lon);
```

## **Parameters**

- `boat_lat` *(double)*: Latitude of the boat in decimal degrees.
- `boat_lon` *(double)*: Longitude of the boat in decimal degrees.
- `waypoint_lat` *(double)*: Latitude of the waypoint in decimal degrees.
- `waypoint_lon` *(double)*: Longitude of the waypoint in decimal degrees.

## **Returns**

- *(double)*: The azimuth angle (0° to 360°) indicating the direction from the boat to the waypoint relative to true North.

## **Example Usage**

### **Code Example**

```c
#include <stdio.h>
#include <math.h>
#define PI 3.14159265358979323846

double calculate_azimuth(double boat_lat, double boat_lon, double waypoint_lat, double waypoint_lon) {
    double dLon = (waypoint_lon - boat_lon) * (PI / 180.0);
    double lat1 = boat_lat * (PI / 180.0);
    double lat2 = waypoint_lat * (PI / 180.0);

    double y = sin(dLon) * cos(lat2);
    double x = cos(lat1) * sin(lat2) - sin(lat1) * cos(lat2) * cos(dLon);
    double azimuth = atan2(y, x) * (180.0 / PI);

    return fmod(azimuth + 360.0, 360.0);
}

int main() {
    double boat_lat = 48.8566, boat_lon = 2.3522;
    double waypoint_lat = 48.8570, waypoint_lon = 2.3530;
    
    double azimuth = calculate_azimuth(boat_lat, boat_lon, waypoint_lat, waypoint_lon);
    printf("Azimuth to waypoint: %.2f°\n", azimuth);
    return 0;
}
```

### **Expected Output**

```
Azimuth to waypoint: 45.72°
```

---

# `define_no_go_zone` Function Documentation

## **Description**

The `define_no_go_zone` function determines the no-go zone for a sailboat based on the wind direction. This zone represents the range of angles where the boat **cannot sail effectively** due to the wind being too direct. The zone typically extends ±45° from the wind direction.

## **Prototype**

```c
void define_no_go_zone(double wind_vane, double *min_angle, double *max_angle);
```

## **Parameters**

| Parameter    | Type     | Description |
|--------------|----------|-------------|
| `wind_vane`  | `double` | Wind direction **relative to North** (0-360°). |
| `min_angle` | `double*` | Pointer to store the **minimum angle** of the no-go zone. |
| `max_angle` | `double*` | Pointer to store the **maximum angle** of the no-go zone. |

## **Returns**

- This function **modifies** the values of `min_angle` and `max_angle` via pointers.

## **Example Usage**

```c
#include <stdio.h>
#include <math.h>

#define NoGoZone 45.0

void define_no_go_zone(double wind_vane, double *min_angle, double *max_angle) {
    double wind_abs = fmod(wind_vane + 360.0, 360.0);
    *min_angle = fmod(wind_abs - NoGoZone + 360.0, 360.0);
    *max_angle = fmod(wind_abs + NoGoZone, 360.0);
}

int main() {
    double wind_vane = 180.0;  // Wind is coming from the South
    double min_angle, max_angle;

    define_no_go_zone(wind_vane, &min_angle, &max_angle);

    printf("Wind Direction: %.2f°\n", wind_vane);
    printf("No-Go Zone: [%.2f° to %.2f°]\n", min_angle, max_angle);

    return 0;
}
```

### **Expected Output**

```
Wind Direction: 180.00°
No-Go Zone: [135.00° to 225.00°]
```

---

# `is_in_no_go_zone` Function Documentation

## **Description**

The `is_in_no_go_zone` function checks whether a given azimuth angle lies within a predefined no-go zone. This no-go zone is determined based on the wind direction, and it is the area where the boat cannot sail effectively.

## **Parameters**

- **`azimuth` (double)**: The azimuth angle representing the boat’s current heading.
- **`min_angle` (double)**: The minimum angle of the no-go zone.
- **`max_angle` (double)**: The maximum angle of the no-go zone.

## **Returns**

- **Returns `1`** if the azimuth is within the no-go zone.
- **Returns `0`** if the azimuth is outside the no-go zone.

## **Example Usage**

```c
#include <stdio.h>
#include <math.h>

int is_in_no_go_zone(double azimuth, double min_angle, double max_angle) {
    if (min_angle < max_angle)
        return (azimuth >= min_angle && azimuth <= max_angle);
    else
        return (azimuth >= min_angle || azimuth <= max_angle);
}

int main() {
    double azimuth = 50.0;
    double min_angle = 135.0;
    double max_angle = 225.0;

    if (is_in_no_go_zone(azimuth, min_angle, max_angle)) {
        printf("The azimuth %.2f° is in the no-go zone.\n", azimuth);
    } else {
        printf("The azimuth %.2f° is NOT in the no-go zone.\n", azimuth);
    }

    return 0;
}
```

### **Expected Output**

```
The azimuth 50.00° is NOT in the no-go zone.
```