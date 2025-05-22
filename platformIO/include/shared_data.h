#pragma once
#include "FreeRTOS.h"
#include "semphr.h"

// Structure partagée par toutes les tâches
typedef struct {
    double latitude;
    double longitude;
    double compass;
    double wind_vane;
    double horizontal_tilt;
    double vertical_tilt;
    int targetAngle;
    int targetTension;
    int angleFromNorth;
} SharedData;

extern SharedData sharedData;