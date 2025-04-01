#include <Arduino.h>
#include "gps.hpp"

#define LED_PIN 25  // Broche LED pour Raspberry Pi Pico

GNSS m_GNSS;

void setup()
{
    // Initialisation de la broche LED
    pinMode(LED_PIN, OUTPUT);

    // Initialisation de la communication série
    Serial.begin(115200);
    while (!Serial)
        ; // Attendre que la connexion série soit établie

    m_GNSS.gpsInit();
}

void loop()
{
    while (1)
    {
        m_GNSS.lireFluxGPS();
        delay(1000);
    }
}