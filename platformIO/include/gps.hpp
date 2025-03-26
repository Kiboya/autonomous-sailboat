#include <Wire.h>

#define ZED_F9P_I2C_ADDRESS 0x42 // Adresse I2C par défaut
//#define ZED_F9P_I2C_ADDRESS 0x21 // Adresse I2C trouvée par scan I2C

void scanI2C();
void activeUBX();
void configurerTrameNAV_PVT_I2C();
void lireFluxGPS();
void gpsInit();
void envoiPositionGpsVersPico();