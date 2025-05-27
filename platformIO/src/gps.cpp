#include "gps.hpp"
#include "shared_data.h"

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

void GNSS::activeUBX_RTK()
{
    // 1. On active UBX en sortie I2C
    uint8_t enableUBX_I2C[] = {
        0xB5, 0x62,             // Sync chars
        0x06, 0x1A,             // Class = CFG, ID = CFG-PRT (0x06 0x1A)
        0x08, 0x00,             // Payload length = 8
        0x01, 0x00, 0x00, 0x00, // Enable UBX only (bit 0), disable NMEA/RTCM
        0x00, 0x00, 0x00, 0x00, // Reserved
        0x23, 0x71              // Checksum
    };

    I2C1Instance.beginTransmission(ZED_F9P_I2C_ADDRESS);
    for (size_t i = 0; i < sizeof(enableUBX_I2C); i++)
        I2C1Instance.write(enableUBX_I2C[i]);
    I2C1Instance.endTransmission();

    delay(500);

    // 2. Puis on active le message UBX-RXM-RTCM (class 0x02, ID 0x32) sur I2C (port ID = 0x03)
    uint8_t enable_RXM_RTCM_on_I2C[] = {
        0xB5, 0x62,             // Sync
        0x06, 0x01,             // CFG-MSG
        0x03, 0x00,             // Length = 3
        0x02, 0x32, 0x01,       // Class = 0x02, ID = 0x32, Rate on I2C = 1
        0x3F, 0x4C              // Checksum
    };

    I2C1Instance.beginTransmission(ZED_F9P_I2C_ADDRESS);
    for (size_t i = 0; i < sizeof(enable_RXM_RTCM_on_I2C); i++)
        I2C1Instance.write(enable_RXM_RTCM_on_I2C[i]);
    I2C1Instance.endTransmission();

    Serial.println("Sortie UBX activée et UBX-RXM-RTCM activé sur I2C.");
}

void GNSS::lireFluxGPS()
{
    if (myGNSS.getPVT())
    {
        if (myGNSS.getRELPOSNED()) // Si on reçois des corrections RTK
        {
            uint8_t fixType = myGNSS.packetUBXNAVPVT->data.fixType;
            uint8_t carrSoln = myGNSS.packetUBXNAVRELPOSNED->data.flags.bits.carrSoln;

            Serial.print("FixType: ");
            Serial.print(fixType);
            Serial.print(" | Carrier Solution: ");
            Serial.print(carrSoln);
            Serial.print(" -> ");

            if (fixType == 5 && carrSoln == 2)
            {
                Serial.println("RTK Fixed");
            }
            else if (fixType >= 4 && carrSoln == 1)
            {
                Serial.println("RTK Float");
            }
            else
            {
                Serial.println("Pas de RTK");
            }
        }

        double latitude = myGNSS.getLatitude() / 1e7;  // Latitude ...
        double longitude = myGNSS.getLongitude() / 1e7; // ... et longitude en degrés.
        double altitude = myGNSS.getAltitude() / 1e3;  // Altitude en mètres

        Serial.print("Latitude : ");
        Serial.print(latitude, 7);
        sharedData.latitude = latitude; // Stocker la latitude dans sharedData
        Serial.print(", Longitude : ");
        Serial.print(longitude, 7);
        sharedData.longitude = longitude;
        Serial.print(", Altitude : ");
        Serial.print(altitude, 2);
        sharedData.altitude = altitude;

        Serial.println(" m");
    }
    else
    {
       Serial.println("Pas de données GNSS disponibles.");
    }
    delay(1000);
}

void GNSS::configurerUART_RX2()
{
    // UBX-CFG-PRT for UART2: Set in protocol in = RTCM3, out = 0
    uint8_t cfg_prt_uart2[] = {
        0xB5, 0x62,             // Sync chars
        0x06, 0x00,             // Class = CFG, ID = PRT
        0x14, 0x00,             // Length = 20 bytes
        0x01,                   // PortID = 1 (UART2)
        0x00,                   // Reserved
        0x00, 0x00,             // txReady (not used)
        0xD0, 0x08, 0x00, 0x00, // mode (8N1, 38400 bauds default)
        0x01, 0xC2, 0x00, 0x00, // baudRate = 115200 (0x01C200) ou 19200 (0x4B00) ou 38400 (0x9600), adapter si besoin
        0x00, 0x02,             // inProtoMask = RTCM3 (bit 1)
        0x00, 0x00,             // outProtoMask = 0 (pas de sortie)
        0x00, 0x00,             // flags
        0x00, 0x00              // reserved
    };

    // Calcul du checksum
    uint8_t ckA = 0, ckB = 0;
    for (size_t i = 2; i < sizeof(cfg_prt_uart2); i++) {
        ckA += cfg_prt_uart2[i];
        ckB += ckA;
    }

    I2C1Instance.beginTransmission(ZED_F9P_I2C_ADDRESS);
    for (size_t i = 0; i < sizeof(cfg_prt_uart2); i++)
        I2C1Instance.write(cfg_prt_uart2[i]);
    I2C1Instance.write(ckA);
    I2C1Instance.write(ckB);
    I2C1Instance.endTransmission();

    Serial.println("Configuration de RX2 pour réception RTCM terminée.");
}

void GNSS::gpsInit()
{
    delay(2000);

    I2C1Instance.begin();

    if (!myGNSS.begin(I2C1Instance, ZED_F9P_I2C_ADDRESS))
    {
        Serial.println("Erreur : Impossible de communiquer avec le ZED-F9P !");
        // while (1); // Bloquer si échec
    }

    Serial.println("Initialisation du GPS par I2C terminée.");
    delay(1000);


    scanI2C(); // A garder pour debug, si rien ne marche, peut être utile ...
    activeUBX_RTK();

    configurerUART_RX2();
}