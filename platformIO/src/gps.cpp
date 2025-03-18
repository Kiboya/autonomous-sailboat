#include "gps.hpp"

TwoWire I2C1Instance(i2c1, 2, 3);

void init()
{
    I2C1Instance.begin();
}

void envoiPositionGpsVersPico()
{
    uint8_t data[128];

    I2C1Instance.beginTransmission(ZED_F9P_I2C_ADDRESS);
    I2C1Instance.write(0xFF);
    I2C1Instance.endTransmission(false); // Requête sans stop

    int bytesRead = I2C1Instance.requestFrom(ZED_F9P_I2C_ADDRESS, 128);
    if (bytesRead > 0)
    {
        for (int i = 0; i < bytesRead; i++)
        {
            data[i] = I2C1Instance.read();
        }

        // Afficher les données brutes
        Serial.print("Données reçues : ");

        for (int i = 0; i < bytesRead; i++)
        {
            Serial.print(data[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
    else
    {
        Serial.println("Aucune donnée reçue.");
    }
}