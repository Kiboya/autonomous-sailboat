#include <Arduino.h>
#include "FreeRTOS.h"
#include "task.h"
#include "example.h"
#include "pathPlanification.h"
#include "shared_data.h"

#define LED_PIN 25  // Broche LED pour Raspberry Pi Pico

// Déclaration des tâches
void TaskBlink(void *pvParameters);
void exampleTask(void *pvParameters);
void pathFinding(void *pvParameters);

void setup() {
    Serial.begin(115200);

    // Création des tâches FreeRTOS
    xTaskCreate(
        TaskBlink,        // Fonction de la tâche
        "LED Task",       // Nom de la tâche
        1024,             // Taille de la pile
        NULL,             // Paramètre
        1,                // Priorité
        NULL              // Handle de tâche (inutile ici)
    );

    xTaskCreate(
      exampleTask,        // Fonction de la tâche
      "exampleTask",       // Nom de la tâche
      1024,             // Taille de la pile
      NULL,             // Paramètre
      1,                // Priorité
      NULL              // Handle de tâche (inutile ici)
  );

  xTaskCreate(
    pathFinding,        // Fonction de la tâche
    "pathPlanification",       // Nom de la tâche
    1024,             // Taille de la pile
    NULL,             // Paramètre
    1,                // Priorité
    NULL              // Handle de tâche (inutile ici)
);

    // Démarrer le planificateur FreeRTOS (optionnel sur Arduino)
    //vTaskStartScheduler();
}

void loop() {
    // Rien ici, car FreeRTOS gère les tâches

}

// Tâche pour faire clignoter la LED
void TaskBlink(void *pvParameters) {
    pinMode(2, OUTPUT);
    while (1) {
        digitalWrite(2, HIGH);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Attendre 1s
        digitalWrite(2, LOW);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Attendre 1s
    }
}

void exampleTask(void *pvParameters) {
    while (1) {
        helloWorld();
        vTaskDelay(pdMS_TO_TICKS(10000)); // Attendre 1s
    }
}


void pathFinding(void *pvParameters) {
    pinMode(3, OUTPUT);
    int i= 0;
    while (1) {
        i++;
        double boat_lat = 48.8566, boat_lon = 2.3522;
        double waypoint_lat = 48.8570, waypoint_lon = 2.3530;
        double horizontal_tilt = 0.0, vertical_tilt = 0.0;
        double compass = 90.0, wind_vane = 0.0;
        Serial.printf("Boat Latitude: %.4f\n", boat_lat);
        Serial.printf("Boat Longitude: %.4f\n", boat_lon);
        Serial.printf("Waypoint Latitude: %.4f\n", waypoint_lat);
        Serial.printf("Waypoint Longitude: %.4f\n", waypoint_lon);
        Serial.printf("Horizontal Tilt: %.2f\n", horizontal_tilt);
        Serial.printf("Vertical Tilt: %.2f\n", vertical_tilt);
        Serial.printf("Compass: %.2f\n", compass);
        Serial.printf("Wind Vane: %.2f\n", wind_vane);
        double direction = calculate_direction(boat_lat, boat_lon, waypoint_lat, waypoint_lon, horizontal_tilt, vertical_tilt, compass, wind_vane);
        Serial.printf("Iteration: %d\n", i);
        Serial.printf("Optimal direction: %.2f°\n", direction);
        digitalWrite(3, LOW);
        vTaskDelay(pdMS_TO_TICKS(10)); // Attendre 1s
        digitalWrite(3, HIGH);
    }
}