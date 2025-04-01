#include "gps.hpp"

TwoWire I2C1Instance(i2c1, 2, 3);

GNSS::GNSS() : myGNSS()
{

}

GNSS::~GNSS()
{
    I2C1Instance.end();
    Serial.println("GNSS instance destroyed.");
}

void GNSS::scanI2C()
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

void GNSS::activeUBX()
{
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

void GNSS::configurerTrameNAV_PVT_I2C()
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

void GNSS::lireFluxGPS()
{
    if (myGNSS.getPVT())
    {
        double latitude = myGNSS.getLatitude() / 1e7;  // Latitude ...
        double longitude = myGNSS.getLongitude() / 1e7; // ... et longitude en degrés.
        double altitude = myGNSS.getAltitude() / 1e3;  // Altitude en mètres

        Serial.print("Latitude : ");
        Serial.print(latitude, 7);
        Serial.print(", Longitude : ");
        Serial.print(longitude, 7);
        Serial.print(", Altitude : ");
        Serial.print(altitude, 2);
        Serial.println(" m");
    }
    else
    {
       Serial.println("Pas de données GNSS disponibles.");
    }
    delay(1000);
}


void GNSS::gpsInit()
{
    delay(2000);

    I2C1Instance.begin();

    if (!myGNSS.begin(I2C1Instance, ZED_F9P_I2C_ADDRESS))
    {
        Serial.println("Erreur : Impossible de communiquer avec le ZED-F9P !");
        while (1); // Bloquer si échec
    }
    Serial.println("Initialisation I2C terminée.");

    delay(1000);


    scanI2C(); // XXX - Potentiellement à garder pour debug, si rien ne marche, peut être utile ...

    //activeUBX(); // Pour recevoir les trames UBX et non les trames NMEA

    //configurerTrameNAV_PVT_I2C(); // Pour recevoir les trames UBX contenant lattitude, longitude et altitude (NAV_PVT)
}