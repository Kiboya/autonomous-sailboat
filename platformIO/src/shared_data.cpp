#include "shared_data.h"

SharedData sharedData = {};  // Initialisation des valeurs à 0
SemaphoreHandle_t sharedDataMutex = xSemaphoreCreateMutex();
