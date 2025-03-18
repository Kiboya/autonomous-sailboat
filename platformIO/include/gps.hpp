#include <Wire.h>

#define ZED_F9P_I2C_ADDRESS 0x42 // Adresse I2C par défaut

TwoWire I2C1Instance(i2c1, 2, 3);

class GPS
{
    public :

        GPS() {};

        ~GPS() {};

        void envoiPositionGpsVersPico();


};