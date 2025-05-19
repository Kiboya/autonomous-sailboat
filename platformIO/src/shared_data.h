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
} SharedData;

extern SharedData sharedData;
extern SemaphoreHandle_t sharedDataMutex;
