// #include "debug.hpp"
// #include <SparkFun_u-blox_GNSS_Arduino_Library.h>


// TwoWire I2C1Instance(i2c1, 2, 3);
// SFE_UBLOX_GNSS myGNSS;

// void scanI2C()
// {
//     Serial.println("Scanning I2C bus...");
//     for (uint8_t address = 1; address < 127; address++)
//     {
//         I2C1Instance.beginTransmission(address);
//         if (I2C1Instance.endTransmission() == 0)
//         {
//             Serial.print("Device found at address 0x");
//             Serial.println(address, HEX);
//         }
//     }
//     Serial.println("I2C scan complete.");
// }

// void gpsInit()
// {
//     I2C1Instance.begin();
//     if (!myGNSS.begin(I2C1Instance, ZED_F9P_I2C_ADDRESS))
//     {
//         Serial.println("Erreur : Impossible de communiquer avec le ZED-F9P !");
//         while (1);
//     }
//     Serial.println("ZED-F9P initialisé !");
// }

// void lireFluxGPS()
// {
//     // Lire les données GNSS du ZED-F9P
//     if (myGNSS.getPVT())
//     {
//         double latitude = myGNSS.getLatitude() / 1e7;
//         double longitude = myGNSS.getLongitude() / 1e7;
//         double altitude = myGNSS.getAltitude() / 1e3;

//         Serial.print("Lat: ");
//         Serial.print(latitude, 7);
//         Serial.print(" Lon: ");
//         Serial.print(longitude, 7);
//         Serial.print(" Alt: ");
//         Serial.print(altitude, 2);
//         Serial.println(" m");
//     }
//   delay(1000);
// }