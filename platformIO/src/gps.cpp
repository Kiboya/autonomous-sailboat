#include "gps.hpp"

TwoWire I2C1Instance(i2c1, 2, 3);

void scanI2C()
{
    Serial.println("Scanning I2C bus...");
    for (uint8_t address = 1; address < 127; address++)
    {
        I2C1Instance.beginTransmission(address);
        if (I2C1Instance.endTransmission() == 0)
        {
            Serial.print("Device found at address 0x");
            Serial.println(address, HEX);
        }
    }
    Serial.println("I2C scan complete.");
}

void gpsInit()
{
    I2C1Instance.begin();
    Serial.println("Initialisation I2C terminée.");

    // Scan the I2C bus to confirm the GPS module's address
    scanI2C();

    // Configuration UBX : Exemple pour activer les trames NAV-PVT
    uint8_t ubxConfig[] = {
        0xB5, 0x62,       // Sync chars
        0x06, 0x01,       // Class and ID (CFG-MSG)
        0x03, 0x00,       // Payload length
        0x01, 0x07, 0x01, // Payload (NAV-PVT, I2C)
        0x13, 0xBE        // Checksum (CK_A, CK_B)
    };

    I2C1Instance.beginTransmission(ZED_F9P_I2C_ADDRESS);
    for (size_t i = 0; i < sizeof(ubxConfig); i++)
    {
        I2C1Instance.write(ubxConfig[i]);
    }
    if (I2C1Instance.endTransmission() == 0) // Vérifier si la transmission est réussie
    {
        Serial.println("Configuration UBX envoyée avec succès.");
    }
    else
    {
        Serial.println("Erreur lors de l'envoi de la configuration UBX.");
    }

    // Attendre pour s'assurer que le GPS traite la configuration
    delay(1000);
}

void envoiPositionGpsVersPico()
{
    uint8_t data[128];

    I2C1Instance.beginTransmission(ZED_F9P_I2C_ADDRESS);
    I2C1Instance.write(0xFF);
    if (I2C1Instance.endTransmission(false) != 0) // Vérifier si la requête est réussie
    {
        Serial.println("Erreur lors de la requête I2C. Vérifiez l'adresse ou la connexion.");
        return;
    }

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