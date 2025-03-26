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

void activeUBX()
{
    // Active le protocole UBX sur I2C (CFG-I2COUTPROT-UBX)
    uint8_t enableUBX_I2C[] = {
        0xB5, 0x62,       // Sync chars
        0x06, 0x1A,       // Class (CFG) and ID (I2COUTPROT)
        0x08, 0x00,       // Payload length
        0x01, 0x00, 0x00, 0x00, // UBX enabled, NMEA, RTCM disabled
        0x00, 0x00, 0x00, 0x00, // Reserved
        0x23, 0x71        // Checksum (à recalculer si tu modifies)
    };

    // Envoi CFG-I2COUTPROT-UBX
    I2C1Instance.beginTransmission(ZED_F9P_I2C_ADDRESS);
    for (size_t i = 0; i < sizeof(enableUBX_I2C); i++)
    {
        I2C1Instance.write(enableUBX_I2C[i]);
    }
    if (I2C1Instance.endTransmission() == 0)
    {
        Serial.println("CFG-I2COUTPROT-UBX envoyé avec succès.");
    }
    else
    {
        Serial.println("Erreur lors de l'envoi de CFG-I2COUTPROT-UBX.");
    }

    delay(1000);
}

void configurerTrameNAV_PVT_I2C()
{
    uint8_t cfgNavPvt[] = {
        0xB5, 0x62,       // Sync chars
        0x06, 0x01,       // Class (CFG) and ID (MSG)
        0x08, 0x00,       // Payload length (8 bytes)
        0x01, 0x07,       // Message ID: NAV-PVT
        0x00, 0x00, 0x00, // Rate on UART1, UART2, USB (désactivé)
        0x01, 0x00,       // Activer sur I2C
        0x10, 0x4E        // Checksum (CK_A, CK_B) → recalculé pour ce message
    };

    I2C1Instance.beginTransmission(ZED_F9P_I2C_ADDRESS);
    for (size_t i = 0; i < sizeof(cfgNavPvt); i++)
    {
        I2C1Instance.write(cfgNavPvt[i]);
    }

    if (I2C1Instance.endTransmission() == 0)
    {
        Serial.println("NAV-PVT activé sur I2C.");
    }
    else
    {
        Serial.println("Erreur lors de la configuration de NAV-PVT sur I2C.");
    }

    delay(1000);
}

void lireFluxGPS()
{
    uint8_t buffer[128];
    int bytesRead = I2C1Instance.requestFrom(ZED_F9P_I2C_ADDRESS, sizeof(buffer));

    if (bytesRead > 0)
    {
        Serial.print("Données UBX reçues : ");
        for (int i = 0; i < bytesRead; i++)
        {
            Serial.print(buffer[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
    else
    {
        Serial.println("Aucune donnée UBX reçue.");
    }
}


void gpsInit()
{
    delay(2000);

    I2C1Instance.begin();
    Serial.println("Initialisation I2C terminée.");
    delay(1000);

    // Scan the I2C bus to confirm the GPS module's address
    scanI2C();

    activeUBX();

    configurerTrameNAV_PVT_I2C();
}

void envoiPositionGpsVersPico()
{
    lireFluxGPS();
}