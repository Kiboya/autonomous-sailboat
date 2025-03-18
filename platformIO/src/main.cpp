#include <Arduino.h>
#include "FreeRTOS.h"
#include "task.h"
#include <example.h>

#define LED_PIN 25  // Broche LED pour Raspberry Pi Pico

// Déclaration des tâches
void TaskBlink(void *pvParameters);
void exampleTask(void *pvParameters);

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