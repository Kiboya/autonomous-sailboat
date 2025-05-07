#include <Arduino.h>
#include "FreeRTOS.h"
#include "task.h"
#include "gps.hpp"
#include "xbee.hpp"

GNSS m_GNSS;


// Déclaration des tâches
void GpsVersPicoTask(void *pvParameters);
void XbeeTask(void *pvParameters);

void setup()
{
    // Initialisation de la communication série
    Serial.begin(115200);
    while (!Serial)
        ; // Attendre que la connexion série soit établie

    m_GNSS.gpsInit();
    xbee_setup();

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

void XbeeTask(void *pvParameters)
{
    while (1)
    {
        xbee_run();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}