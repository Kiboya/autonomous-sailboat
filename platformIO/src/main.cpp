#include <Arduino.h>
#include "FreeRTOS.h"
#include "task.h"
#include <example.h>
#include "cmps12.h"
#include "qmc5883l.h"

#define LED_PIN 25  // Broche LED pour Raspberry Pi Pico

// Déclaration des tâches existantes
void TaskBlink(void *pvParameters);
void exampleTask(void *pvParameters);
// Nouvelle tâche pour les capteurs
void sensorTask(void *pvParameters);
void i2cScanTask(void *pvParameters);

// Création des instances TwoWire pour chaque capteur
// (Attention : selon votre carte, il faudra adapter la création des instances)
TwoWire I2C1Instance(i2c1, 2, 3); // Pour le CMPS12 : instance i2c1, SDA = GP2, SCL = GP3
TwoWire I2C0Instance(i2c0, 4, 5); // Pour le QMC5883L : instance i2c0, SDA = GP4, SCL = GP5

// Instanciation des capteurs avec leurs bus I2C respectifs
CMPS12 cmps12(I2C1Instance, 0x60);
QMC5883L qmc5883l(I2C0Instance, 0x0D);

void setup() {
    Serial.begin(115200);
    I2C1Instance.begin();

    // Création des tâches FreeRTOS
    xTaskCreate(
        TaskBlink,        // Fonction de la tâche
        "LED Task",       // Nom de la tâche
        1024,             // Taille de la pile
        NULL,             // Paramètre
        1,                // Priorité
        NULL              // Handle de tâche
    );

    xTaskCreate(
      exampleTask,
      "Example Task",
      1024,
      NULL,
      1,
      NULL
    );

    xTaskCreate(
      sensorTask,
      "Sensor Task",
      2048,  // Taille de pile augmentée pour la gestion I2C et la calibration
      NULL,
      1,
      NULL
    );

    // xTaskCreate(i2cScanTask, "I2C Scan Task",1024, NULL, 1, NULL);

    // vTaskStartScheduler();  // Sur Arduino, le scheduler démarre automatiquement
}

void loop() {
    // Rien ici, FreeRTOS gère les tâches
}

// Tâche pour faire clignoter la LED
void TaskBlink(void *pvParameters) {
    pinMode(2, OUTPUT);
    while (1) {
        digitalWrite(2, HIGH);
        vTaskDelay(pdMS_TO_TICKS(1000));
        digitalWrite(2, LOW);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Tâche d'exemple existante
void exampleTask(void *pvParameters) {
    while (1) {
        helloWorld();
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

// Nouvelle tâche pour la gestion des capteurs
void sensorTask(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(10000));
    // Initialisation des capteurs
    cmps12.begin(); 
    qmc5883l.begin();
    
    // Calibration du CMPS12
    Serial.println("Starting CMPS12 Calibration...");
    cmps12.startCalibration();
    Serial.println("Veuillez faire pivoter le capteur pendant 30 secondes...");
    vTaskDelay(pdMS_TO_TICKS(10000));
    cmps12.endCalibration();
    Serial.println("Calibration terminée. Début de la lecture des données.");
    
    while (1) {
         // Lecture des données du CMPS12
         uint16_t compassBearing16 = cmps12.readCompassBearing();
         int8_t pitch = cmps12.readPitch();
         int8_t roll = cmps12.readRoll();
         uint8_t calibrationState = cmps12.readCalibrationState();
         
         Serial.println("Inclinaison de l'appareil (avant/arrière) :");
         Serial.print("Pitch angle: ");
         Serial.println(pitch);
         Serial.println("Rotation (gauche/droite) :");
         Serial.print("Roll angle: ");
         Serial.println(roll);
         Serial.print("Calibration State: ");
         Serial.println(calibrationState);
         Serial.println("-------------------------------------------------------");
         
         // Lecture et calcul de l'orientation via QMC5883L
         float headingQMC = qmc5883l.getHeading();
         
         Serial.print("Direction (CMPS12) : ");
         Serial.print(compassBearing16 / 10);
         Serial.print(".");
         Serial.print(compassBearing16 % 10);
         Serial.println(" degrees");
         Serial.print("Direction (QMC5883L) : ");
         Serial.print(headingQMC);
         Serial.println(" degres");
         Serial.println("-------------------------------------------------------");
         
         vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// // Fonction de scan du bus I2C
// void scanI2CBus(TwoWire &wire) {
//   Serial.println("Début du scan I2C...");
//   byte count = 0;
//   for (byte address = 1; address < 127; address++) {
//     wire.beginTransmission(address);
//     byte error = wire.endTransmission();
//     if (error == 0) {
//       Serial.print("Périphérique trouvé à l'adresse 0x");
//       if (address < 16)
//         Serial.print("0");
//       Serial.println(address, HEX);
//       count++;
//     }
//   }
//   if (count == 0) {
//     Serial.println("Aucun périphérique I2C trouvé.");
//   } else {
//     Serial.print(count);
//     Serial.println(" périphérique(s) trouvé(s).");
//   }
// }

// // Tâche FreeRTOS pour scanner le bus I2C périodiquement
// void i2cScanTask(void *pvParameters) {
//   // Initialisation du bus I2C (ici l'instance par défaut Wire)
//   I2C1Instance.begin();
//   while (1) {
//     scanI2CBus(I2C1Instance);
//     vTaskDelay(pdMS_TO_TICKS(10000)); // Re-scan toutes les 10 secondes
//   }
// }