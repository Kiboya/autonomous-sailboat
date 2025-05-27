#ifndef SHAREDDATA_H
#define SHAREDDATA_H

// Structure partagée par toutes les tâches
typedef struct {
    double latitude;
    double longitude;
    double waypoint_lat;
    double waypoint_lon;
    double compass;
    double wind_vane;
    double horizontal_tilt;
    double vertical_tilt;
    int targetAngle;
    int targetTension;
    int angleFromNorth;
} SharedData;

extern SharedData sharedData;

#endif