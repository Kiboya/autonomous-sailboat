#include <Arduino.h>
#include "FreeRTOS.h"
#include "task.h"
#include "pathPlanification.h"
#include <example.h>
#include "cmps12.h"
#include "qmc5883l.h"
#include <servoControl.h>
#include <xbeeImpl.h>


#define LED_PIN 25 // Broche LED pour Raspberry Pi Pico

servoControl boat;

// Déclaration des tâches existantes
void TaskBlink(void *pvParameters);
void exampleTask(void *pvParameters);
void pathFinding(void *pvParameters);
// Nouvelle tâche pour les capteurs
void sensorTask(void *pvParameters);
void i2cScanTask(void *pvParameters);

// Création des instances TwoWire pour chaque capteur
// (Attention : selon votre carte, il faudra adapter la création des instances)
// TwoWire I2C1Instance(i2c1, 2, 3); // Pour le CMPS12 : instance i2c1, SDA = GP2, SCL = GP3
TwoWire I2C0Instance(i2c0, 4, 5); // Pour le QMC5883L : instance i2c0, SDA = GP4, SCL = GP5

// Instanciation des capteurs avec leurs bus I2C respectifs
CMPS12 cmps12(I2C0Instance, 0x60);
// QMC5883L qmc5883l(I2C0Instance, 0x0D);

void setup()
{
    Serial.begin(115200);

    // Création des tâches FreeRTOS
    // xTaskCreate(
    //     TaskBlink,        // Fonction de la tâche
    //     "LED Task",       // Nom de la tâche
    //     1024,             // Taille de la pile
    //     NULL,             // Paramètre
    //     1,                // Priorité
    //     NULL              // Handle de tâche
    // );

    xTaskCreate(
        TaskBlink,  // Fonction de la tâche
        "LED Task", // Nom de la tâche
        1024,       // Taille de la pile
        NULL,       // Paramètre
        1,          // Priorité
        NULL        // Handle de tâche (inutile ici)
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
    // vTaskStartScheduler();
}

void loop()
{
    // Rien ici, car FreeRTOS gère les tâches

}

// Tâche pour faire clignoter la LED
void TaskBlink(void *pvParameters)
{
    pinMode(2, OUTPUT);
    while (1)
    {
        digitalWrite(2, HIGH);
        vTaskDelay(pdMS_TO_TICKS(1000));
        digitalWrite(2, LOW);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Nouvelle tâche pour la gestion des capteurs
void sensorTask(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(10000));
    // Initialisation des capteurs
    cmps12.begin(); 
    // qmc5883l.begin();
    
    // Calibration du CMPS12
    Serial.println("Starting CMPS12 Calibration...");
    cmps12.startCalibration();
    Serial.println("Veuillez faire pivoter le capteur pendant 30 secondes...");
    vTaskDelay(pdMS_TO_TICKS(30000));
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
         
        //  // Lecture et calcul de l'orientation via QMC5883L
        //  float headingQMC = qmc5883l.getHeading();
         
         Serial.print("Direction (CMPS12) : ");
         Serial.print(compassBearing16 / 10);
         Serial.print(".");
         Serial.print(compassBearing16 % 10);
         Serial.println(" degrees");
        //  Serial.print("Direction (QMC5883L) : ");
        //  Serial.print(headingQMC);
        //  Serial.println(" degres");
        //  Serial.println("-------------------------------------------------------");
         
         vTaskDelay(pdMS_TO_TICKS(500));

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
        double compass = 90.0, wind_vane = 180.0;
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