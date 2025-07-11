#include <Arduino.h>
#include "FreeRTOS.h"
#include "task.h"
#include "gps.hpp"
#include "pathPlanification.h"
#include "cmps12.h"
#include "qmc5883l.h"
#include "shared_data.h"
#include "servoControl.h"
#include "xbeeImpl.h"

#define LED_PIN 25 // Broche LED pour Raspberry Pi Pico

GNSS m_GNSS;

servoControl boat;
xbeeImpl xbee;
SharedData sharedData;

// Déclaration des tâches existantes
void TaskBlink(void *pvParameters);
void exampleTask(void *pvParameters);
void controlTask(void *pvParameters);
void pathFinding(void *pvParameters);
void GpsVersPicoTask(void *pvParameters);
void XbeeTask(void *pvParameters);
// Nouvelle tâche pour les capteurs
void sensorTask(void *pvParameters);
void i2cScanTask(void *pvParameters);

// Création des instances TwoWire pour chaque capteur
// (Attention : selon votre carte, il faudra adapter la création des instances)
// TwoWire I2C1Instance(i2c1, 2, 3); // Pour le QMC5883L : instance i2c1, SDA = GP2, SCL = GP3
TwoWire I2C0Instance(i2c0, 4, 5); // Pour le CMPS12 : instance i2c0, SDA = GP4, SCL = GP5

// Instanciation des capteurs avec leurs bus I2C respectifs
CMPS12 cmps12(I2C0Instance, 0x60);
// QMC5883L qmc5883l(I2C0Instance, 0x0D);

void setup()
{
  Serial.begin(115200);
  while (!Serial)
        ; // Attendre que la connexion série soit établie

  m_GNSS.gpsInit();

  xTaskCreate(
    TaskBlink,  // Fonction de la tâche
    "LED Task", // Nom de la tâche
    1024,       // Taille de la pile
    NULL,       // Paramètre
    1,          // Priorité
    NULL        // Handle de tâche (inutile ici)
  );

  xTaskCreate(
    controlTask,  // Fonction de la tâche
    "control task", // Nom de la tâche
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

  xTaskCreate(
    sensorTask,  // Fonction de la tâche
    "LED Task", // Nom de la tâche
    1024,       // Taille de la pile
    NULL,       // Paramètre
    1,          // Priorité
    NULL        // Handle de tâche (inutile ici)
  );

  xTaskCreate(
    GpsVersPicoTask,        // Fonction de la tâche
    "GpsVersPicoTask",      // Nom de la tâche
    1024,                   // Taille de la pile
    NULL,                   // Paramètre
    1,                      // Priorité
    NULL                    // Handle de tâche (inutile ici)
  );

  xTaskCreate(
    XbeeTask,               // Fonction de la tâche
    "XbeeTask",             // Nom de la tâche
    1024,                   // Taille de la pile
    NULL,                   // Paramètre
    1,                      // Priorité
    NULL                    // Handle de tâche (inutile ici)
  );

  // Démarrer le planificateur FreeRTOS (optionnel sur Arduino)
  // vTaskStartScheduler();
}

void loop()
{
    // Rien ici, car FreeRTOS gère les tâches
}

void GpsVersPicoTask(void *pvParameters)
{
    while (1)
    {
        m_GNSS.lireFluxGPS();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
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

void XbeeTask(void *pvParameters) {
  // Initialisation de l'interface série pour XBee
  xbee.initialize();
  while (1)
  {
    xbee.read();
    xbee.send(sharedData);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void controlTask(void *pvParameters) {
  while (1)
  {
    boat.servo_control(xbee);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// Nouvelle tâche pour la gestion des capteurs
void sensorTask(void *pvParameters) {
    vTaskDelay(pdMS_TO_TICKS(10000));
    // Initialisation des capteurs
    cmps12.begin();
    // qmc5883l.begin();

    // // Calibration du CMPS12
    // Serial.println("Starting CMPS12 Calibration...");
    // cmps12.startCalibration();
    // Serial.println("Veuillez faire pivoter le capteur pendant 30 secondes...");
    // vTaskDelay(pdMS_TO_TICKS(30000));
    // cmps12.endCalibration();
    // Serial.println("Calibration terminée. Début de la lecture des données.");

    while (1) {
        // Lecture des données du CMPS12
        uint16_t compassBearing16 = cmps12.readCompassBearing();
        int8_t pitch = cmps12.readPitch();
        int8_t roll = cmps12.readRoll();
        uint8_t calibrationState = cmps12.readCalibrationState();

        sharedData.horizontal_tilt = roll;
        sharedData.vertical_tilt = pitch;
        sharedData.angleFromNorth = compassBearing16 / 10;

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
    // Create static instance of LaylinePathPlanner
    static LaylinePathPlanner laylinePlanner;
    int iteration = 0;
    
    while (1) {
        iteration++;
        
        // Use real sensor data when available, otherwise use test data
        double boat_lat = sharedData.latitude != 0.0 ? sharedData.latitude : 48.8566;
        double boat_lon = sharedData.longitude != 0.0 ? sharedData.longitude : 2.3522;
        double waypoint_lat = sharedData.waypoint_lat != 0.0 ? sharedData.waypoint_lat : 47.253699;
        double waypoint_lon = sharedData.waypoint_lon != 0.0 ? sharedData.waypoint_lon : -1.370199;
        
        double compass = sharedData.angleFromNorth != 0 ? sharedData.angleFromNorth : 90.0;
        double wind_vane = sharedData.wind_vane != 0.0 ? sharedData.wind_vane : 180.0; // Wind direction relative to boat
        double wind_speed = sharedData.wind_speed != 0.0 ? sharedData.wind_speed : 5.0; // Wind speed

        // Get current time in seconds (convert from millis)
        double current_time = millis() / 1000.0;
        
        Serial.printf("=== Path Planning Iteration %d ===\n", iteration);
        Serial.printf("Boat Position: %.6f, %.6f\n", boat_lat, boat_lon);
        Serial.printf("Waypoint: %.6f, %.6f\n", waypoint_lat, waypoint_lon);
        Serial.printf("Compass: %.1f°, Wind: %.1f° @ %.1f m/s\n", compass, wind_vane, wind_speed);
        
        // Calculate optimal direction using LaylinePathPlanner
        double direction = laylinePlanner.calculate_direction(
            boat_lat, boat_lon, waypoint_lat, waypoint_lon,
            compass, wind_vane, wind_speed, current_time
        );
        
        Serial.printf("Optimal direction: %.1f°\n", direction);
        Serial.println("================================\n");
        
        // Update shared data with calculated direction
        sharedData.targetAngle = (int)round(direction);
        vTaskDelay(pdMS_TO_TICKS(500));
        
    }
}
