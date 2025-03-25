#include <Wire.h>

//#define ZED_F9P_I2C_ADDRESS 0x42 // Adresse I2C par défaut
#define ZED_F9P_I2C_ADDRESS 0x21 // Adresse I2C trouvée par scan I2C

void gpsInit();
void envoiPositionGpsVersPico();