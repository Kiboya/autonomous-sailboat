# 🧠 Sharing Variables Between Tasks with `shared_data.h` on Raspberry Pi Pico 2 (FreeRTOS + PlatformIO)

## 1️⃣ The `shared_data.h` File (Already Exists in the Git Project)

This file serves as a central point for declaring **shared variables** between multiple tasks. It contains the external declarations. Make sure to add the variables you use in your tasks/functions.

```c
#pragma once
#include "FreeRTOS.h"
#include "semphr.h"

// Structure shared by all tasks
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
```

---

## 2️⃣ The `shared_data.c` File (Already Exists in the Git Project)

```c
#include "shared_data.h"

SharedData sharedData = {};  // Initialize all values to 0
SemaphoreHandle_t sharedDataMutex = xSemaphoreCreateMutex();
```

---

## 3️⃣ Include `shared_data.h` in Your Task Files

In each task (`.cpp` file), simply include `shared_data.h` to access the shared variables.

```c
#include "shared_data.h"
```

---

## 4️⃣ Protect Concurrent Access

Use **mutexes** to avoid conflicts when multiple tasks access shared variables at the same time. This must be done for all variables that will be used by other tasks.

### Example:

```c
if (xSemaphoreTake(sharedDataMutex, pdMS_TO_TICKS(100))) {
    sharedData.compass = xxxxx;
    xSemaphoreGive(sharedDataMutex);
}
```

In `main.c`, use the following code to safely retrieve values from any task:

```c
SharedData data;

if (xSemaphoreTake(sharedDataMutex, pdMS_TO_TICKS(100))) {
    data = sharedData;
    xSemaphoreGive(sharedDataMutex);
}
Serial.printf("Shared Value: %.4f\n", data.compass);
```

---

## 5️⃣ Remember to Add New Variables to the `SharedData` Struct in `shared_data.h`

```c
typedef struct {
    double latitude;
    double longitude;
    double compass;
    double wind_vane;
    double horizontal_tilt;
    double vertical_tilt;
} SharedData;
```

